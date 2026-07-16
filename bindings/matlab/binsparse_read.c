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
