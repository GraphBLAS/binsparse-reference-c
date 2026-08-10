/*
 * SPDX-FileCopyrightText: 2026 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * Write a MATLAB CSC sparse matrix directly as row-sorted Binsparse COO.
 *
 * Usage:
 *   binsparse_write_ssmc_coo(filename, A, Zeros, group, json, compression)
 *
 * The conversion uses a counting scatter rather than the generic COO qsort.
 * Only reordered columns and non-ISO values are materialized.  Row indices are
 * generated directly into bounded HDF5 write buffers from the row endpoints.
 */

#include "matlab_bsp_helpers.h"
#include "matlab_bsp_strings.h"
#include "mex.h"
#include <binsparse/binsparse_all.h>
#include <binsparse/hdf5_wrapper.h>
#include <cJSON/cJSON.h>
#include <complex.h>
#include <hdf5.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COO_WRITE_CHUNK_BYTES ((size_t) (1024 * 1024))

typedef struct value_analysis_t {
  bool is_iso;
  bool fits_float32;
  double iso_real;
  double iso_imag;
} value_analysis_t;

static bsp_type_t index_type_for_extent(size_t extent) {
  return bsp_pick_integer_type(extent == 0 ? 0 : extent - 1);
}

static char* required_string(const mxArray* value, const char* name,
                             bool allow_empty) {
  if (!mxIsChar(value) || mxGetM(value) > 1 ||
      (!allow_empty && mxIsEmpty(value))) {
    mexErrMsgIdAndTxt("BinSparse:InvalidString",
                      "%s must be a character row vector", name);
  }
  char* string = mxArrayToString(value);
  if (!string) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError", "Failed to read %s", name);
  }
  return string;
}

static int compression_level(const mxArray* value) {
  if (!mxIsNumeric(value) || mxIsComplex(value) ||
      mxGetNumberOfElements(value) != 1) {
    mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                      "Compression level must be a real numeric scalar");
  }
  double level = mxGetScalar(value);
  if (level < 0.0 || level > 9.0 || level != (double) (int) level) {
    mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                      "Compression level must be an integer from 0 to 9");
  }
  return (int) level;
}

static bool value_fits_float32(double real, double imag, bool is_complex) {
  if ((double) (float) real != real) {
    return false;
  }
  return !is_complex || (double) (float) imag == imag;
}

static value_analysis_t analyze_values(const matlab_csc_t* a,
                                       const matlab_csc_t* z) {
  value_analysis_t analysis = {.is_iso = a->nnz + z->nnz > 0,
                               .fits_float32 = true,
                               .iso_real = a->nnz > 0 ? a->values[0] : 0.0,
                               .iso_imag =
                                   a->nnz > 0 && a->imag_values
                                       ? a->imag_values[0]
                                       : 0.0};

  for (size_t i = 0; i < a->nnz; i++) {
    double real = a->values[i];
    double imag = a->imag_values ? a->imag_values[i] : 0.0;
    if (i > 0 &&
        (real != analysis.iso_real || imag != analysis.iso_imag)) {
      analysis.is_iso = false;
    }
    if (!value_fits_float32(real, imag, a->is_complex)) {
      analysis.fits_float32 = false;
    }
  }

  if (z->nnz > 0 &&
      (analysis.iso_real != 0.0 || analysis.iso_imag != 0.0)) {
    analysis.is_iso = false;
  }
  return analysis;
}

static bsp_type_t output_value_type(const matlab_csc_t* a,
                                    value_analysis_t analysis) {
  if (a->is_complex) {
    return analysis.fits_float32 ? BSP_COMPLEX_FLOAT32
                                 : BSP_COMPLEX_FLOAT64;
  }
  return analysis.fits_float32 ? BSP_FLOAT32 : BSP_FLOAT64;
}

static void write_output_value(bsp_array_t values, size_t index, double real,
                               double imag) {
  switch (values.type) {
  case BSP_FLOAT32:
    ((float*) values.data)[index] = (float) real;
    break;
  case BSP_FLOAT64:
    ((double*) values.data)[index] = real;
    break;
  case BSP_COMPLEX_FLOAT32:
    ((float _Complex*) values.data)[index] =
        (float) real + (float) imag * I;
    break;
  case BSP_COMPLEX_FLOAT64:
    ((double _Complex*) values.data)[index] = real + imag * I;
    break;
  default:
    break;
  }
}

static void write_column(bsp_array_t columns, size_t index, size_t column) {
  bsp_array_write(columns, index, column);
}

static void fill_indices(void* data, bsp_type_t type, size_t offset,
                         size_t count, size_t value) {
  switch (type) {
  case BSP_UINT8: {
    uint8_t* x = (uint8_t*) data;
    for (size_t k = 0; k < count; k++)
      x[offset + k] = (uint8_t) value;
    break;
  }
  case BSP_UINT16: {
    uint16_t* x = (uint16_t*) data;
    for (size_t k = 0; k < count; k++)
      x[offset + k] = (uint16_t) value;
    break;
  }
  case BSP_UINT32: {
    uint32_t* x = (uint32_t*) data;
    for (size_t k = 0; k < count; k++)
      x[offset + k] = (uint32_t) value;
    break;
  }
  case BSP_UINT64: {
    uint64_t* x = (uint64_t*) data;
    for (size_t k = 0; k < count; k++)
      x[offset + k] = (uint64_t) value;
    break;
  }
  default:
    break;
  }
}

static bsp_error_t write_generated_rows(hid_t parent, const uint64_t* row_ends,
                                        size_t nrows, size_t nnz,
                                        bsp_type_t index_type,
                                        int compression) {
  hsize_t dimensions[1] = {(hsize_t) nnz};
  hid_t file_space = H5Screate_simple(1, dimensions, NULL);
  if (file_space == H5I_INVALID_HID) {
    return BSP_ERROR_IO;
  }

  hid_t properties = H5Pcreate(H5P_DATASET_CREATE);
  if (properties == H5I_INVALID_HID) {
    H5Sclose(file_space);
    return BSP_ERROR_IO;
  }

  size_t element_size = bsp_type_size(index_type);
  size_t capacity = COO_WRITE_CHUNK_BYTES / element_size;
  if (capacity == 0)
    capacity = 1;
  if (nnz > 0 && capacity > nnz)
    capacity = nnz;

  if (nnz > 0) {
    hsize_t chunk[1] = {(hsize_t) capacity};
    if (H5Pset_chunk(properties, 1, chunk) < 0 ||
        (compression > 0 &&
         H5Pset_deflate(properties, (unsigned) compression) < 0)) {
      H5Pclose(properties);
      H5Sclose(file_space);
      return BSP_ERROR_IO;
    }
  }

  hid_t dataset = H5Dcreate2(parent, "indices_0",
                             bsp_get_hdf5_standard_type(index_type), file_space,
                             H5P_DEFAULT, properties, H5P_DEFAULT);
  H5Pclose(properties);
  if (dataset == H5I_INVALID_HID) {
    H5Sclose(file_space);
    return BSP_ERROR_IO;
  }

  if (nnz == 0) {
    H5Dclose(dataset);
    H5Sclose(file_space);
    return BSP_SUCCESS;
  }

  void* buffer = mxMalloc(capacity * element_size);
  if (!buffer) {
    H5Dclose(dataset);
    H5Sclose(file_space);
    return BSP_ERROR_MEMORY;
  }

  size_t row = 0;
  size_t written = 0;
  bsp_error_t error = BSP_SUCCESS;
  while (written < nnz) {
    size_t buffered = 0;
    while (buffered < capacity && written + buffered < nnz) {
      while (row < nrows && written + buffered >= row_ends[row])
        row++;
      if (row == nrows) {
        error = BSP_ERROR_INTERNAL;
        break;
      }
      size_t available = (size_t) row_ends[row] - (written + buffered);
      size_t room = capacity - buffered;
      size_t count = available < room ? available : room;
      fill_indices(buffer, index_type, buffered, count, row);
      buffered += count;
    }
    if (error != BSP_SUCCESS)
      break;

    hsize_t start[1] = {(hsize_t) written};
    hsize_t count[1] = {(hsize_t) buffered};
    hid_t memory_space = H5Screate_simple(1, count, NULL);
    if (memory_space == H5I_INVALID_HID ||
        H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, count,
                            NULL) < 0 ||
        H5Dwrite(dataset, bsp_get_hdf5_native_type(index_type), memory_space,
                 file_space, H5P_DEFAULT, buffer) < 0) {
      if (memory_space != H5I_INVALID_HID)
        H5Sclose(memory_space);
      error = BSP_ERROR_IO;
      break;
    }
    H5Sclose(memory_space);
    written += buffered;
  }

  mxFree(buffer);
  H5Dclose(dataset);
  H5Sclose(file_space);
  return error;
}

static bsp_error_t write_descriptor(hid_t parent, bsp_matrix_t descriptor,
                                    const char* json) {
  cJSON* user_json = json ? cJSON_Parse(json) : NULL;
  if (!user_json)
    user_json = cJSON_CreateObject();
  if (!user_json)
    return BSP_ERROR_MEMORY;

  char* descriptor_json = bsp_generate_json(descriptor, user_json);
  cJSON_Delete(user_json);
  if (!descriptor_json)
    return BSP_ERROR_MEMORY;
  bsp_error_t error =
      bsp_write_attribute(parent, (char*) "binsparse", descriptor_json);
  free(descriptor_json);
  return error;
}

static bsp_error_t write_coo_file(const char* filename, const char* group,
                                  bsp_matrix_t matrix,
                                  const uint64_t* row_ends, const char* json,
                                  int compression) {
  hid_t file = H5I_INVALID_HID;
  hid_t parent = H5I_INVALID_HID;
  bool have_group = group && group[0] != '\0';

  H5dont_atexit();
  if (!have_group) {
    file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    parent = file;
  } else {
    file = access(filename, F_OK) == 0
               ? H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)
               : H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file != H5I_INVALID_HID)
      parent = H5Gcreate1(file, group, H5P_DEFAULT);
  }
  if (file == H5I_INVALID_HID || parent == H5I_INVALID_HID) {
    if (file != H5I_INVALID_HID)
      H5Fclose(file);
    return BSP_ERROR_IO;
  }

  bsp_error_t error =
      bsp_write_array(parent, "values", matrix.values, compression);
  if (error == BSP_SUCCESS) {
    error = write_generated_rows(parent, row_ends, matrix.nrows, matrix.nnz,
                                 matrix.indices_0.type, compression);
  }
  if (error == BSP_SUCCESS) {
    error = bsp_write_array(parent, "indices_1", matrix.indices_1, compression);
  }
  if (error == BSP_SUCCESS) {
    error = write_descriptor(parent, matrix, json);
  }

  if (have_group)
    H5Gclose(parent);
  H5Fclose(file);
  return error;
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  (void) plhs;
  bsp_lock_mex_module();
  if (nrhs != 6) {
    mexErrMsgIdAndTxt(
        "BinSparse:InvalidArgs",
        "Usage: binsparse_write_ssmc_coo(filename, A, Zeros, group, json, "
        "compression)");
  }
  if (nlhs != 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs", "No output is returned");
  }

  char* filename = required_string(prhs[0], "filename", false);
  char* group = required_string(prhs[3], "group", true);
  char* json = NULL;
  if (!mxIsEmpty(prhs[4])) {
    json = bsp_mx_to_utf8(prhs[4]);
    if (!json) {
      mxFree(group);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:InvalidJSON",
                        "JSON must be a valid character vector");
    }
  }
  int compression = compression_level(prhs[5]);

  const mxArray* mx_a = prhs[1];
  const mxArray* mx_z = mxIsEmpty(prhs[2]) ? NULL : prhs[2];
  if (!mxIsSparse(mx_a) || !mxIsDouble(mx_a)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix",
                      "A must be a sparse double matrix");
  }
  if (mx_z &&
      (!mxIsSparse(mx_z) || mxIsComplex(mx_z) ||
       (!mxIsDouble(mx_z) && !mxIsLogical(mx_z)))) {
    mexErrMsgIdAndTxt("BinSparse:InvalidZeros",
                      "Zeros must be a real sparse double or logical matrix");
  }
  if (mx_z &&
      (mxGetM(mx_a) != mxGetM(mx_z) || mxGetN(mx_a) != mxGetN(mx_z))) {
    mexErrMsgIdAndTxt("BinSparse:DimensionMismatch",
                      "A and Zeros must have matching dimensions");
  }

  matlab_csc_t a = {0};
  matlab_csc_t z = {0};
  if (extract_matlab_csc(mx_a, &a) != 0 ||
      (mx_z && extract_matlab_csc(mx_z, &z) != 0)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix",
                      "Failed to access MATLAB CSC arrays");
  }
  if (a.nnz > SIZE_MAX - z.nnz) {
    mexErrMsgIdAndTxt("BinSparse:SizeOverflow", "Stored entry count overflows");
  }
  size_t nnz = a.nnz + z.nnz;

  uint64_t* positions =
      a.nrows > 0 ? (uint64_t*) mxCalloc(a.nrows, sizeof(uint64_t)) : NULL;
  if (a.nrows > 0 && !positions) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate row counts");
  }
  for (size_t p = 0; p < a.nnz; p++)
    positions[a.rowind[p]]++;
  for (size_t p = 0; p < z.nnz; p++)
    positions[z.rowind[p]]++;

  uint64_t running = 0;
  for (size_t row = 0; row < a.nrows; row++) {
    uint64_t count = positions[row];
    positions[row] = running;
    running += count;
  }
  if (running != nnz) {
    mxFree(positions);
    mexErrMsgIdAndTxt("BinSparse:InternalError", "Row counts do not match nnz");
  }

  value_analysis_t analysis = analyze_values(&a, &z);
  bsp_matrix_t output;
  bsp_construct_default_matrix_t_allocator(&output, bsp_matlab_allocator);
  output.nrows = a.nrows;
  output.ncols = a.ncols;
  output.nnz = nnz;
  output.is_iso = analysis.is_iso;
  output.format = BSP_COOR;
  output.structure = BSP_GENERAL;

  bsp_type_t value_type = output_value_type(&a, analysis);
  bsp_error_t error = bsp_construct_array_t_allocator(
      &output.values, output.is_iso ? 1 : nnz, value_type,
      bsp_matlab_allocator);
  if (error == BSP_SUCCESS) {
    error = bsp_construct_array_t_allocator(
        &output.indices_1, nnz, index_type_for_extent(a.ncols),
        bsp_matlab_allocator);
  }
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(&output);
    mxFree(positions);
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate COO output arrays");
  }
  output.indices_0.size = nnz;
  output.indices_0.type = index_type_for_extent(a.nrows);
  output.indices_0.data = nnz > 0 ? output.indices_1.data : NULL;
  output.indices_0.allocator = bsp_matlab_view_allocator;

  if (output.is_iso) {
    write_output_value(output.values, 0, analysis.iso_real, analysis.iso_imag);
  }

  const char* conversion_error = NULL;
  for (size_t column = 0; column < a.ncols && !conversion_error; column++) {
    mwIndex ap = a.colptr[column];
    mwIndex aend = a.colptr[column + 1];
    mwIndex zp = z.colptr ? z.colptr[column] : 0;
    mwIndex zend = z.colptr ? z.colptr[column + 1] : 0;
    while (ap < aend || zp < zend) {
      bool take_a = zp >= zend ||
                    (ap < aend && a.rowind[ap] < z.rowind[zp]);
      bool take_z = ap >= aend ||
                    (zp < zend && z.rowind[zp] < a.rowind[ap]);
      if (!take_a && !take_z) {
        conversion_error = "Duplicate indices between A and Zeros";
        break;
      }

      size_t row = take_a ? a.rowind[ap] : z.rowind[zp];
      size_t destination = (size_t) positions[row]++;
      write_column(output.indices_1, destination, column);
      if (!output.is_iso) {
        double real = take_a ? a.values[ap] : 0.0;
        double imag = take_a && a.imag_values ? a.imag_values[ap] : 0.0;
        write_output_value(output.values, destination, real, imag);
      }
      if (take_a)
        ap++;
      else
        zp++;
    }
  }

  if (conversion_error) {
    bsp_destroy_array_t(&output.values);
    bsp_destroy_array_t(&output.indices_1);
    mxFree(positions);
    mxFree(json);
    mxFree(group);
    mxFree(filename);
    mexErrMsgIdAndTxt("BinSparse:DuplicateIndex", "%s", conversion_error);
  }

  error = write_coo_file(filename, group, output, positions, json, compression);
  bsp_destroy_array_t(&output.values);
  bsp_destroy_array_t(&output.indices_1);
  mxFree(positions);
  if (json)
    mxFree(json);
  mxFree(group);
  mxFree(filename);

  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:WriteError", "Failed to write COO matrix: %s",
                      bsp_get_error_string(error));
  }
}
