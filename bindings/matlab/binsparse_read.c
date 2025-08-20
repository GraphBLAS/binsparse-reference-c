/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_read.c - Read Binsparse matrices into MATLAB
 *
 * This MEX function reads Binsparse format matrices and returns them
 * as MATLAB structs compatible with bsp_matrix_create.
 *
 * Usage in MATLAB/Octave:
 *   matrix = binsparse_read(filename)
 *   matrix = binsparse_read(filename, group)
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <string.h>

/**
 * Convert bsp_array_t to MATLAB array
 */
mxArray* bsp_array_to_matlab(const bsp_array_t* array) {
  if (array->data == NULL || array->size == 0) {
    // Return empty array
    return mxCreateDoubleMatrix(0, 1, mxREAL);
  }

  mxArray* mx_array = NULL;

  switch (array->type) {
  case BSP_FLOAT64:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxDOUBLE_CLASS, mxREAL);
    memcpy(mxGetPr(mx_array), array->data, array->size * sizeof(double));
    break;

  case BSP_FLOAT32:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxSINGLE_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(float));
    break;

  case BSP_UINT64:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxUINT64_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(uint64_t));
    break;

  case BSP_UINT32:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxUINT32_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(uint32_t));
    break;

  case BSP_UINT16:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxUINT16_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(uint16_t));
    break;

  case BSP_UINT8:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxUINT8_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(uint8_t));
    break;

  case BSP_INT64:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxINT64_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(int64_t));
    break;

  case BSP_INT32:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxINT32_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(int32_t));
    break;

  case BSP_INT16:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxINT16_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(int16_t));
    break;

  case BSP_INT8:
    mx_array = mxCreateNumericMatrix(array->size, 1, mxINT8_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(int8_t));
    break;

  case BSP_BINT8:
    // Treat BSP_BINT8 as UINT8 as suggested
    mx_array = mxCreateNumericMatrix(array->size, 1, mxUINT8_CLASS, mxREAL);
    memcpy(mxGetData(mx_array), array->data, array->size * sizeof(int8_t));
    break;

  case BSP_COMPLEX_FLOAT64: {
    mx_array = mxCreateNumericMatrix(array->size, 1, mxDOUBLE_CLASS, mxCOMPLEX);
    double* in_data =
        (double*) array->data; // Treat as array of adjacent real/imag pairs
    double* real_data = mxGetPr(mx_array);
    double* imag_data = mxGetPi(mx_array);
    for (size_t i = 0; i < array->size; i++) {
      real_data[i] = in_data[2 * i];     // Real part
      imag_data[i] = in_data[2 * i + 1]; // Imaginary part
    }
    break;
  }

  case BSP_COMPLEX_FLOAT32: {
    mx_array = mxCreateNumericMatrix(array->size, 1, mxSINGLE_CLASS, mxCOMPLEX);
    float* in_data =
        (float*) array->data; // Treat as array of adjacent real/imag pairs
    float* real_data = (float*) mxGetData(mx_array);
    float* imag_data = (float*) mxGetImagData(mx_array);
    for (size_t i = 0; i < array->size; i++) {
      real_data[i] = in_data[2 * i];     // Real part
      imag_data[i] = in_data[2 * i + 1]; // Imaginary part
    }
    break;
  }

  default:
    // Fallback: create empty array
    mx_array = mxCreateDoubleMatrix(0, 1, mxREAL);
    mexWarnMsgIdAndTxt("BinSparse:UnsupportedType",
                       "Unsupported array type %d, returning empty array",
                       (int) array->type);
    break;
  }

  return mx_array;
}

/**
 * Convert bsp_matrix_t to MATLAB struct
 */
mxArray* bsp_matrix_to_matlab_struct(const bsp_matrix_t* matrix) {
  const char* field_names[] = {
      "values", "indices_0", "indices_1", "pointers_to_1", "nrows",
      "ncols",  "nnz",       "is_iso",    "format",        "structure"};

  mxArray* mx_struct = mxCreateStructMatrix(1, 1, 10, field_names);

  // Convert arrays
  mxSetField(mx_struct, 0, "values", bsp_array_to_matlab(&matrix->values));
  mxSetField(mx_struct, 0, "indices_0",
             bsp_array_to_matlab(&matrix->indices_0));
  mxSetField(mx_struct, 0, "indices_1",
             bsp_array_to_matlab(&matrix->indices_1));
  mxSetField(mx_struct, 0, "pointers_to_1",
             bsp_array_to_matlab(&matrix->pointers_to_1));

  // Convert scalar fields
  mxSetField(mx_struct, 0, "nrows",
             mxCreateDoubleScalar((double) matrix->nrows));
  mxSetField(mx_struct, 0, "ncols",
             mxCreateDoubleScalar((double) matrix->ncols));
  mxSetField(mx_struct, 0, "nnz", mxCreateDoubleScalar((double) matrix->nnz));
  mxSetField(mx_struct, 0, "is_iso", mxCreateLogicalScalar(matrix->is_iso));

  // Convert format string
  mxSetField(mx_struct, 0, "format",
             mxCreateString(bsp_get_matrix_format_string(matrix->format)));

  // Convert structure string
  mxSetField(mx_struct, 0, "structure",
             mxCreateString(bsp_get_structure_string(matrix->structure)));

  return mx_struct;
}

/**
 * Main MEX function entry point
 */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  char* filename = NULL;
  char* group = NULL;
  bsp_matrix_t matrix;
  bsp_error_t error;

  // Check input arguments
  if (nrhs < 1 || nrhs > 2) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: matrix = binsparse_read(filename [, group])");
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs", "Too many output arguments");
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

  // Get optional group name
  if (nrhs == 2) {
    if (!mxIsChar(prhs[1])) {
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:InvalidGroup",
                        "Group name must be a string");
    }

    group = mxArrayToString(prhs[1]);
    if (!group) {
      mxFree(filename);
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert group string");
    }
  }

  // Read the matrix using Binsparse
  error = bsp_read_matrix(&matrix, filename, group);

  if (error != BSP_SUCCESS) {
    // Clean up
    if (filename)
      mxFree(filename);
    if (group)
      mxFree(group);

    mexErrMsgIdAndTxt("BinSparse:ReadError", "Failed to read matrix: %s",
                      bsp_get_error_string(error));
  }

  // Convert to MATLAB struct
  plhs[0] = bsp_matrix_to_matlab_struct(&matrix);

  // Clean up
  bsp_destroy_matrix_t(&matrix);
  if (filename)
    mxFree(filename);
  if (group)
    mxFree(group);
}
