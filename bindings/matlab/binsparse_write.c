/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_write.c - Write MATLAB structs to Binsparse format
 *
 * This MEX function writes MATLAB structs (compatible with bsp_matrix_create)
 * to Binsparse format files.
 *
 * Usage in MATLAB/Octave:
 *   binsparse_write(filename, matrix)
 *   binsparse_write(filename, matrix, group)
 *   binsparse_write(filename, matrix, group, json_string)
 *   binsparse_write(filename, matrix, group, json_string, compression_level)
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <string.h>

/**
 * Convert MATLAB array to bsp_array_t
 */
bsp_error_t matlab_to_bsp_array(const mxArray* mx_array, bsp_array_t* array) {
  if (mxIsEmpty(mx_array)) {
    bsp_construct_default_array_t(array);
    return BSP_SUCCESS;
  }

  size_t size = mxGetNumberOfElements(mx_array);
  mxClassID class_id = mxGetClassID(mx_array);
  bool is_complex = mxIsComplex(mx_array);

  // Determine BSP type from MATLAB class
  bsp_type_t bsp_type;
  size_t element_size;

  if (is_complex) {
    if (class_id == mxDOUBLE_CLASS) {
      bsp_type = BSP_COMPLEX_FLOAT64;
      element_size = sizeof(double _Complex);
    } else if (class_id == mxSINGLE_CLASS) {
      bsp_type = BSP_COMPLEX_FLOAT32;
      element_size = sizeof(float _Complex);
    } else {
      return BSP_INVALID_TYPE;
    }
  } else {
    switch (class_id) {
    case mxDOUBLE_CLASS:
      bsp_type = BSP_FLOAT64;
      element_size = sizeof(double);
      break;
    case mxSINGLE_CLASS:
      bsp_type = BSP_FLOAT32;
      element_size = sizeof(float);
      break;
    case mxUINT64_CLASS:
      bsp_type = BSP_UINT64;
      element_size = sizeof(uint64_t);
      break;
    case mxUINT32_CLASS:
      bsp_type = BSP_UINT32;
      element_size = sizeof(uint32_t);
      break;
    case mxUINT16_CLASS:
      bsp_type = BSP_UINT16;
      element_size = sizeof(uint16_t);
      break;
    case mxUINT8_CLASS:
      bsp_type = BSP_UINT8;
      element_size = sizeof(uint8_t);
      break;
    case mxINT64_CLASS:
      bsp_type = BSP_INT64;
      element_size = sizeof(int64_t);
      break;
    case mxINT32_CLASS:
      bsp_type = BSP_INT32;
      element_size = sizeof(int32_t);
      break;
    case mxINT16_CLASS:
      bsp_type = BSP_INT16;
      element_size = sizeof(int16_t);
      break;
    case mxINT8_CLASS:
      bsp_type = BSP_INT8;
      element_size = sizeof(int8_t);
      break;
    default:
      return BSP_INVALID_TYPE;
    }
  }

  // Allocate BSP array
  bsp_error_t error = bsp_construct_array_t(array, size, bsp_type);
  if (error != BSP_SUCCESS) {
    return error;
  }

  // Copy data
  if (is_complex) {
    // Handle complex numbers: interleave real/imaginary parts
    if (class_id == mxDOUBLE_CLASS) {
      double* real_data = mxGetPr(mx_array);
      double* imag_data = mxGetPi(mx_array);
      double* out_data = (double*) array->data;
      for (size_t i = 0; i < size; i++) {
        out_data[2 * i] = real_data[i];     // Real part
        out_data[2 * i + 1] = imag_data[i]; // Imaginary part
      }
    } else { // mxSINGLE_CLASS
      float* real_data = (float*) mxGetData(mx_array);
      float* imag_data = (float*) mxGetImagData(mx_array);
      float* out_data = (float*) array->data;
      for (size_t i = 0; i < size; i++) {
        out_data[2 * i] = real_data[i];     // Real part
        out_data[2 * i + 1] = imag_data[i]; // Imaginary part
      }
    }
  } else {
    // Simple copy for real types
    memcpy(array->data, mxGetData(mx_array), size * element_size);
  }

  return BSP_SUCCESS;
}

/**
 * Convert MATLAB struct to bsp_matrix_t
 */
bsp_error_t matlab_struct_to_bsp_matrix(const mxArray* mx_struct,
                                        bsp_matrix_t* matrix) {
  bsp_construct_default_matrix_t(matrix);

  // Extract and convert arrays
  mxArray* values_field = mxGetField(mx_struct, 0, "values");
  mxArray* indices_0_field = mxGetField(mx_struct, 0, "indices_0");
  mxArray* indices_1_field = mxGetField(mx_struct, 0, "indices_1");
  mxArray* pointers_to_1_field = mxGetField(mx_struct, 0, "pointers_to_1");

  if (!values_field || !indices_0_field || !indices_1_field ||
      !pointers_to_1_field) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  bsp_error_t error;
  error = matlab_to_bsp_array(values_field, &matrix->values);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array(indices_0_field, &matrix->indices_0);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array(indices_1_field, &matrix->indices_1);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array(pointers_to_1_field, &matrix->pointers_to_1);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  // Extract scalar fields
  mxArray* nrows_field = mxGetField(mx_struct, 0, "nrows");
  mxArray* ncols_field = mxGetField(mx_struct, 0, "ncols");
  mxArray* nnz_field = mxGetField(mx_struct, 0, "nnz");
  mxArray* is_iso_field = mxGetField(mx_struct, 0, "is_iso");

  if (!nrows_field || !ncols_field || !nnz_field || !is_iso_field) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->nrows = (size_t) mxGetScalar(nrows_field);
  matrix->ncols = (size_t) mxGetScalar(ncols_field);
  matrix->nnz = (size_t) mxGetScalar(nnz_field);
  matrix->is_iso = mxIsLogicalScalarTrue(is_iso_field);

  // Extract format string
  mxArray* format_field = mxGetField(mx_struct, 0, "format");
  if (!format_field || !mxIsChar(format_field)) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  char* format_str = mxArrayToString(format_field);
  if (!format_str) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->format = bsp_get_matrix_format(format_str);
  mxFree(format_str);

  if (matrix->format == BSP_INVALID_FORMAT) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_FORMAT;
  }

  // Extract structure string
  mxArray* structure_field = mxGetField(mx_struct, 0, "structure");
  if (!structure_field || !mxIsChar(structure_field)) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  char* structure_str = mxArrayToString(structure_field);
  if (!structure_str) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->structure = bsp_get_structure(structure_str);
  mxFree(structure_str);

  if (matrix->structure == BSP_INVALID_STRUCTURE) {
    matrix->structure = BSP_GENERAL; // Default fallback
  }

  return BSP_SUCCESS;
}

/**
 * Main MEX function entry point
 */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  char* filename = NULL;
  char* group = NULL;
  char* json_string = NULL;
  int compression_level = 1; // Default compression
  bsp_matrix_t matrix;
  bsp_error_t error;

  // Check input arguments
  if (nrhs < 2 || nrhs > 5) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: binsparse_write(filename, matrix [, group [, "
                      "json_string [, compression_level]]])");
  }

  if (nlhs > 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "No output arguments expected");
  }

  // Get filename
  if (!mxIsChar(prhs[0])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFilename", "Filename must be a string");
  }

  filename = mxArrayToString(prhs[0]);
  if (!filename) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to convert filename string");
  }

  // Get matrix struct
  if (!mxIsStruct(prhs[1])) {
    mxFree(filename);
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix", "Matrix must be a struct");
  }

  // Convert MATLAB struct to bsp_matrix_t
  error = matlab_struct_to_bsp_matrix(prhs[1], &matrix);
  if (error != BSP_SUCCESS) {
    mxFree(filename);
    mexErrMsgIdAndTxt("BinSparse:ConversionError",
                      "Failed to convert MATLAB struct to matrix: %s",
                      bsp_get_error_string(error));
  }

  // Get optional group name
  if (nrhs >= 3 && !mxIsEmpty(prhs[2])) {
    if (!mxIsChar(prhs[2])) {
      bsp_destroy_matrix_t(&matrix);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:InvalidGroup",
                        "Group name must be a string");
    }

    group = mxArrayToString(prhs[2]);
    if (!group) {
      bsp_destroy_matrix_t(&matrix);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert group string");
    }
  }

  // Get optional JSON string
  if (nrhs >= 4 && !mxIsEmpty(prhs[3])) {
    if (!mxIsChar(prhs[3])) {
      bsp_destroy_matrix_t(&matrix);
      if (group)
        mxFree(group);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:InvalidJSON", "JSON must be a string");
    }

    json_string = mxArrayToString(prhs[3]);
    if (!json_string) {
      bsp_destroy_matrix_t(&matrix);
      if (group)
        mxFree(group);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert JSON string");
    }
  }

  // Get optional compression level
  if (nrhs >= 5 && !mxIsEmpty(prhs[4])) {
    if (!mxIsNumeric(prhs[4]) || mxIsComplex(prhs[4]) ||
        mxGetNumberOfElements(prhs[4]) != 1) {
      bsp_destroy_matrix_t(&matrix);
      if (json_string)
        mxFree(json_string);
      if (group)
        mxFree(group);
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                        "Compression level must be a scalar integer");
    }

    compression_level = (int) mxGetScalar(prhs[4]);
  }

  // Write the matrix using Binsparse
  error =
      bsp_write_matrix(filename, matrix, group, json_string, compression_level);

  // Clean up
  bsp_destroy_matrix_t(&matrix);
  if (json_string)
    mxFree(json_string);
  if (group)
    mxFree(group);
  mxFree(filename);

  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:WriteError", "Failed to write matrix: %s",
                      bsp_get_error_string(error));
  }
}
