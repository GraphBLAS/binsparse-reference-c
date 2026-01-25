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

static const bsp_allocator_t bsp_matlab_allocator = {.malloc = mxMalloc,
                                                     .free = mxFree};

static inline mxClassID get_mxClassID(bsp_type_t type) {
  switch (type) {
  case BSP_UINT8:
    return mxUINT8_CLASS;
  case BSP_UINT16:
    return mxUINT16_CLASS;
  case BSP_UINT32:
    return mxUINT32_CLASS;
  case BSP_UINT64:
    return mxUINT64_CLASS;
  case BSP_INT8:
    return mxINT8_CLASS;
  case BSP_INT16:
    return mxINT16_CLASS;
  case BSP_INT32:
    return mxINT32_CLASS;
  case BSP_INT64:
    return mxINT64_CLASS;
  case BSP_FLOAT32:
    return mxSINGLE_CLASS;
  case BSP_FLOAT64:
    return mxDOUBLE_CLASS;
  case BSP_BINT8: // Treat BSP_BINT8 as UINT8
    return mxUINT8_CLASS;
  case BSP_COMPLEX_FLOAT32:
    return mxSINGLE_CLASS;
  case BSP_COMPLEX_FLOAT64:
    return mxDOUBLE_CLASS;
  default:
    return mxUNKNOWN_CLASS;
  }
}

static inline mxComplexity get_mxComplexity(bsp_type_t type) {
  if (type == BSP_COMPLEX_FLOAT32 || type == BSP_COMPLEX_FLOAT64) {
    return mxCOMPLEX;
  } else {
    return mxREAL;
  }
}

mxArray* bsp_array_to_matlab(bsp_array_t* array) {
  if (array->data == NULL || array->size == 0) {
    // Return empty array of the correct class
    mxClassID class_id = get_mxClassID(array->type);
    if (class_id == mxUNKNOWN_CLASS) {
      class_id = mxDOUBLE_CLASS;
    }
    return mxCreateNumericMatrix(0, 1, class_id, get_mxComplexity(array->type));
  }

  if (get_mxClassID(array->type) == mxUNKNOWN_CLASS) {
    mexWarnMsgIdAndTxt("BinSparse:UnsupportedType",
                       "Unsupported array type %d, returning empty array",
                       (int) array->type);
    return mxCreateNumericMatrix(0, 1, mxDOUBLE_CLASS, mxREAL);
  }

  mxArray* mx_array = NULL;

  // FIXME: do not use seperate real/imag arrays. Use the new MATLAB interleaved
  // complex API.

  if ((array->allocator.malloc == bsp_matlab_allocator.malloc &&
       array->allocator.free == bsp_matlab_allocator.free) &&
      get_mxComplexity(array->type) == mxREAL) {
    // Create mx_array in a zero-copy fashion.

    mx_array = mxCreateNumericMatrix(0, 1, get_mxClassID(array->type),
                                     get_mxComplexity(array->type));

    mxSetData(mx_array, array->data);
    mxSetM(mx_array, array->size);

    array->data = NULL;
    array->size = 0;
  } else {
    mx_array = mxCreateNumericMatrix(array->size, 1, get_mxClassID(array->type),
                                     get_mxComplexity(array->type));

    if (get_mxComplexity(array->type) == mxREAL) {
      memcpy(mxGetData(mx_array), array->data,
             array->size * bsp_type_size(array->type));
    } else {
      if (array->type == BSP_COMPLEX_FLOAT32) {
        float* in_data =
            (float*) array->data; // Treat as array of adjacent real/imag pairs
        float* real_data = (float*) mxGetData(mx_array);
        float* imag_data = (float*) mxGetImagData(mx_array);
        for (size_t i = 0; i < array->size; i++) {
          real_data[i] = in_data[2 * i];     // Real part
          imag_data[i] = in_data[2 * i + 1]; // Imaginary part
        }
      } else {
        double* in_data =
            (double*) array->data; // Treat as array of adjacent real/imag pairs
        double* real_data = mxGetPr(mx_array);
        double* imag_data = mxGetPi(mx_array);
        for (size_t i = 0; i < array->size; i++) {
          real_data[i] = in_data[2 * i];     // Real part
          imag_data[i] = in_data[2 * i + 1]; // Imaginary part
        }
      }
    }
  }

  return mx_array;
}

/**
 * Convert bsp_matrix_t to MATLAB struct
 */
mxArray* bsp_matrix_to_matlab_struct(bsp_matrix_t* matrix) {
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
  error =
      bsp_read_matrix_allocator(&matrix, filename, group, bsp_matlab_allocator);

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
