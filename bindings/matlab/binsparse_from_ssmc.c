/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_from_ssmc.c - Convert SuiteSparse matrix A + Zeros to a Binsparse
 * MATLAB struct.
 *
 * Usage in MATLAB/Octave:
 *   matrix = binsparse_from_ssmc(A)
 *   matrix = binsparse_from_ssmc(A, Zeros)
 *   matrix = binsparse_from_ssmc(A, Zeros, format)
 *   matrix = binsparse_from_ssmc(A, format)
 *
 * Inputs:
 *   A      - sparse or dense matrix (MATLAB)
 *   Zeros  - optional sparse matrix representing explicit zeros (same size as
 * A) format - optional string: default 'COO' for sparse, 'DMAT'/'DVEC' for
 * dense
 *
 * Output:
 *   MATLAB struct compatible with binsparse_write.
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <complex.h>
#include <string.h>

#include "matlab_bsp_helpers.h"

static bool array_uses_allocator(bsp_array_t array, bsp_allocator_t allocator) {
  if (array.size == 0 || array.data == NULL) {
    return true;
  }
  return array.allocator.malloc == allocator.malloc &&
         array.allocator.free == allocator.free;
}

static bool matrix_uses_allocator(const bsp_matrix_t* matrix,
                                  bsp_allocator_t allocator) {
  return array_uses_allocator(matrix->values, allocator) &&
         array_uses_allocator(matrix->indices_0, allocator) &&
         array_uses_allocator(matrix->indices_1, allocator) &&
         array_uses_allocator(matrix->pointers_to_1, allocator);
}

static bsp_error_t construct_array_with_allocator(bsp_array_t* array,
                                                  size_t size, bsp_type_t type,
                                                  bsp_allocator_t allocator) {
  if (size == 0) {
    array->data = NULL;
    array->size = 0;
    array->type = type;
    array->allocator = allocator;
    return BSP_SUCCESS;
  }
  return bsp_construct_array_t_allocator(array, size, type, allocator);
}

static bsp_type_t sparse_value_type(const matlab_csc_t* matrix) {
  return matrix->is_complex ? BSP_COMPLEX_FLOAT64 : BSP_FLOAT64;
}

static bsp_type_t index_type_for_extent(size_t extent) {
  return bsp_pick_integer_type(extent == 0 ? 0 : extent - 1);
}

static bool sparse_values_are_iso(const matlab_csc_t* matrix, double* real,
                                  double* imag) {
  if (matrix->nnz == 0) {
    return false;
  }

  *real = matrix->values[0];
  *imag = matrix->imag_values ? matrix->imag_values[0] : 0.0;
  for (size_t i = 1; i < matrix->nnz; i++) {
    double current_imag = matrix->imag_values ? matrix->imag_values[i] : 0.0;
    if (matrix->values[i] != *real || current_imag != *imag) {
      return false;
    }
  }
  return true;
}

static bool merged_values_are_iso(const matlab_csc_t* a, const matlab_csc_t* z,
                                  double* real, double* imag) {
  if (a->nnz + z->nnz == 0) {
    return false;
  }

  if (a->nnz == 0) {
    *real = 0.0;
    *imag = 0.0;
    return true;
  }

  if (!sparse_values_are_iso(a, real, imag)) {
    return false;
  }
  return z->nnz == 0 || (*real == 0.0 && *imag == 0.0);
}

static void write_sparse_value(const matlab_csc_t* matrix, mwIndex src,
                               bsp_matrix_t* out, uint64_t dst) {
  if (matrix->is_complex) {
    double _Complex* out_values = (double _Complex*) out->values.data;
    double imag = matrix->imag_values ? matrix->imag_values[src] : 0.0;
    out_values[dst] = matrix->values[src] + imag * I;
  } else {
    double* out_values = (double*) out->values.data;
    out_values[dst] = matrix->values[src];
  }
}

static void write_explicit_zero(bsp_matrix_t* out, uint64_t dst) {
  if (out->values.type == BSP_COMPLEX_FLOAT64) {
    double _Complex* out_values = (double _Complex*) out->values.data;
    out_values[dst] = 0.0 + 0.0 * I;
  } else {
    double* out_values = (double*) out->values.data;
    out_values[dst] = 0.0;
  }
}

static void write_iso_value(bsp_matrix_t* out, double real, double imag) {
  if (out->values.type == BSP_COMPLEX_FLOAT64) {
    double _Complex* out_values = (double _Complex*) out->values.data;
    out_values[0] = real + imag * I;
  } else {
    double* out_values = (double*) out->values.data;
    out_values[0] = real;
  }
}

static void build_csc_merged(const matlab_csc_t* a, const matlab_csc_t* z,
                             bsp_matrix_t* out) {
  bsp_error_t error;

  bsp_construct_default_matrix_t_allocator(out, bsp_matlab_allocator);
  out->nrows = a->nrows;
  out->ncols = a->ncols;
  out->nnz = a->nnz + z->nnz;
  out->format = BSP_CSC;
  out->structure = BSP_GENERAL;
  double iso_real = 0.0;
  double iso_imag = 0.0;
  out->is_iso = merged_values_are_iso(a, z, &iso_real, &iso_imag);

  error = construct_array_with_allocator(
      &out->values, out->is_iso ? 1 : out->nnz, sparse_value_type(a),
      bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate values array");
  }

  bsp_type_t row_index_type = index_type_for_extent(out->nrows);
  error = construct_array_with_allocator(&out->indices_1, out->nnz,
                                         row_index_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate indices array");
  }

  bsp_type_t pointer_type = bsp_pick_integer_type(out->nnz);
  error = construct_array_with_allocator(&out->pointers_to_1, out->ncols + 1,
                                         pointer_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate pointers array");
  }

  if (out->is_iso) {
    write_iso_value(out, iso_real, iso_imag);
  }

  uint64_t running = 0;
  bsp_array_write(out->pointers_to_1, 0, running);
  for (mwIndex j = 0; j < a->ncols; j++) {
    mwIndex a_count = a->colptr[j + 1] - a->colptr[j];
    mwIndex z_count = z->colptr[j + 1] - z->colptr[j];
    running += a_count + z_count;
    bsp_array_write(out->pointers_to_1, j + 1, running);
  }

  for (mwIndex j = 0; j < a->ncols; j++) {
    mwIndex a_ptr = a->colptr[j];
    mwIndex a_end = a->colptr[j + 1];
    mwIndex z_ptr = z->colptr[j];
    mwIndex z_end = z->colptr[j + 1];
    uint64_t out_ptr = (uint64_t) a->colptr[j] + (uint64_t) z->colptr[j];

    while (a_ptr < a_end || z_ptr < z_end) {
      if (z_ptr >= z_end ||
          (a_ptr < a_end && a->rowind[a_ptr] < z->rowind[z_ptr])) {
        if (!out->is_iso) {
          write_sparse_value(a, a_ptr, out, out_ptr);
        }
        bsp_array_write(out->indices_1, out_ptr, a->rowind[a_ptr]);
        a_ptr++;
      } else if (a_ptr >= a_end ||
                 (z_ptr < z_end && z->rowind[z_ptr] < a->rowind[a_ptr])) {
        if (!out->is_iso) {
          write_explicit_zero(out, out_ptr);
        }
        bsp_array_write(out->indices_1, out_ptr, z->rowind[z_ptr]);
        z_ptr++;
      } else {
        mexErrMsgIdAndTxt("BinSparse:DuplicateIndex",
                          "Duplicate indices between A and Zeros");
      }
      out_ptr++;
    }

    if (out_ptr != (uint64_t) a->colptr[j + 1] + (uint64_t) z->colptr[j + 1]) {
      mexErrMsgIdAndTxt("BinSparse:InternalError",
                        "Merged column counts do not match");
    }
  }
}

static bsp_matrix_format_t parse_format(int nrhs, const mxArray* prhs[]) {
  const mxArray* format_arg = NULL;
  if (nrhs >= 3 && !mxIsEmpty(prhs[2])) {
    format_arg = prhs[2];
  } else if (nrhs == 2 && mxIsChar(prhs[1])) {
    format_arg = prhs[1];
  }

  if (!format_arg) {
    return BSP_COO;
  }

  if (!mxIsChar(format_arg)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFormat", "Format must be a string");
  }

  char* format_str = mxArrayToString(format_arg);
  if (!format_str) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError", "Failed to read format string");
  }

  bsp_matrix_format_t format = bsp_get_matrix_format(format_str);
  mxFree(format_str);

  if (format != BSP_CSC && format != BSP_CSR && format != BSP_COO &&
      format != BSP_COOR && format != BSP_DMAT && format != BSP_DMATC &&
      format != BSP_DVEC) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFormat",
                      "Supported formats: CSC, CSR, COO, DMAT, DMATC, DVEC");
  }

  return format;
}

static void build_csc_from_a(const matlab_csc_t* a, bsp_matrix_t* out) {
  bsp_error_t error;

  bsp_construct_default_matrix_t_allocator(out, bsp_matlab_allocator);
  out->nrows = a->nrows;
  out->ncols = a->ncols;
  out->nnz = a->nnz;
  out->format = BSP_CSC;
  out->structure = BSP_GENERAL;
  double iso_real = 0.0;
  double iso_imag = 0.0;
  out->is_iso = sparse_values_are_iso(a, &iso_real, &iso_imag);

  error = construct_array_with_allocator(
      &out->values, out->is_iso ? 1 : out->nnz, sparse_value_type(a),
      bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate values array");
  }

  bsp_type_t row_index_type = index_type_for_extent(out->nrows);
  error = construct_array_with_allocator(&out->indices_1, out->nnz,
                                         row_index_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate indices array");
  }

  bsp_type_t pointer_type = bsp_pick_integer_type(out->nnz);
  error = construct_array_with_allocator(&out->pointers_to_1, out->ncols + 1,
                                         pointer_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate pointers array");
  }

  if (out->is_iso) {
    write_iso_value(out, iso_real, iso_imag);
  }

  for (size_t i = 0; i < out->nnz; i++) {
    if (!out->is_iso) {
      write_sparse_value(a, i, out, (uint64_t) i);
    }
    bsp_array_write(out->indices_1, i, a->rowind[i]);
  }

  for (size_t i = 0; i < out->ncols + 1; i++) {
    bsp_array_write(out->pointers_to_1, i, a->colptr[i]);
  }
}

static void build_dense_matrix(const mxArray* mx_a, bsp_matrix_t* out,
                               bsp_matrix_format_t format) {
  if (!mxIsNumeric(mx_a)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix", "Dense input must be numeric");
  }

  bool is_vector = (mxGetM(mx_a) == 1) || (mxGetN(mx_a) == 1);

  if (is_vector && format != BSP_DVEC) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFormat",
                      "Dense vector requires DVEC format");
  }

  if (!is_vector && format != BSP_DMAT && format != BSP_DMATC) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFormat",
                      "Dense matrix requires DMAT or DMATC format");
  }

  bsp_construct_default_matrix_t_allocator(out, bsp_matlab_allocator);
  out->format = format;
  out->structure = BSP_GENERAL;
  out->is_iso = false;

  size_t nrows = mxGetM(mx_a);
  size_t ncols = mxGetN(mx_a);
  size_t total = mxGetNumberOfElements(mx_a);

  if (is_vector) {
    out->nrows = total;
    out->ncols = 1;
  } else {
    out->nrows = nrows;
    out->ncols = ncols;
  }

  out->nnz = total;

  bsp_error_t error =
      matlab_to_bsp_array_allocator(mx_a, &out->values, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to allocate dense values");
  }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  if (nrhs < 1 || nrhs > 3) {
    mexErrMsgIdAndTxt(
        "BinSparse:InvalidArgs",
        "Usage: matrix = binsparse_from_ssmc(A [, Zeros] [, format])");
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs", "Too many output arguments");
  }

  const mxArray* mx_a = prhs[0];
  bsp_matrix_format_t target_format = parse_format(nrhs, prhs);

  if (!mxIsSparse(mx_a)) {
    bsp_matrix_t dense_matrix;
    build_dense_matrix(mx_a, &dense_matrix, target_format);
    plhs[0] = bsp_matrix_to_matlab_struct(&dense_matrix);
    bsp_destroy_matrix_t(&dense_matrix);
    return;
  }

  if (target_format == BSP_DMAT || target_format == BSP_DVEC) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFormat",
                      "Sparse matrix cannot use DMAT/DVEC formats");
  }

  if (!mxIsDouble(mx_a)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix",
                      "A must be a sparse double matrix");
  }

  const mxArray* mx_zeros = NULL;
  if (nrhs >= 2 && mxIsSparse(prhs[1])) {
    mx_zeros = prhs[1];
  }

  if (mx_zeros && (mxIsComplex(mx_zeros) ||
                   (!mxIsDouble(mx_zeros) && !mxIsLogical(mx_zeros)))) {
    mexErrMsgIdAndTxt("BinSparse:InvalidZeros",
                      "Zeros must be a real sparse double or logical matrix");
  }

  if (mx_zeros &&
      (mxGetM(mx_a) != mxGetM(mx_zeros) || mxGetN(mx_a) != mxGetN(mx_zeros))) {
    mexErrMsgIdAndTxt("BinSparse:DimensionMismatch",
                      "A and Zeros must have matching dimensions");
  }

  matlab_csc_t a_csc = {0};
  matlab_csc_t z_csc = {0};

  if (extract_matlab_csc(mx_a, &a_csc) != 0) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix",
                      "Failed to extract CSC data from A");
  }

  bool have_zeros = false;
  if (mx_zeros) {
    if (extract_matlab_csc(mx_zeros, &z_csc) != 0) {
      mexErrMsgIdAndTxt("BinSparse:InvalidZeros",
                        "Failed to extract CSC data from Zeros");
    }
    have_zeros = true;
  }

  bsp_matrix_t csc_matrix;
  if (have_zeros) {
    build_csc_merged(&a_csc, &z_csc, &csc_matrix);
  } else {
    build_csc_from_a(&a_csc, &csc_matrix);
  }

  bsp_matrix_t result = csc_matrix;
  if (target_format != BSP_CSC) {
    result = bsp_convert_matrix_allocator(csc_matrix, target_format,
                                          bsp_matlab_allocator);
    bsp_destroy_matrix_t(&csc_matrix);

    if (result.format != target_format) {
      bsp_destroy_matrix_t(&result);
      mexErrMsgIdAndTxt("BinSparse:ConversionError",
                        "Failed to convert matrix to requested format");
    }
  }

  if (!matrix_uses_allocator(&result, bsp_matlab_allocator)) {
    bsp_matrix_t copied;
    bsp_error_t error =
        bsp_matrix_copy_with_allocator(&result, &copied, bsp_matlab_allocator);
    bsp_destroy_matrix_t(&result);
    if (error != BSP_SUCCESS) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to allocate MATLAB-owned matrix");
    }
    result = copied;
  }

  plhs[0] = bsp_matrix_to_matlab_struct(&result);
  bsp_destroy_matrix_t(&result);
}
