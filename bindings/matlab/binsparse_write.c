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
#include <stdbool.h>
#include <string.h>

#include "matlab_bsp_helpers.h"

// Keep this MEX function loaded for the whole MATLAB session: the HDF5
// library used by libbinsparse installs process-wide state that is not safe
// to tear down when a MEX file is cleared.
static void lock_mex_module(void) {
  if (!mexIsLocked()) {
    mexLock();
  }
}

/**
 * Main MEX function entry point
 */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  lock_mex_module();

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
  error = matlab_struct_to_bsp_matrix_allocator(prhs[1], &matrix,
                                                bsp_matlab_allocator);
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
