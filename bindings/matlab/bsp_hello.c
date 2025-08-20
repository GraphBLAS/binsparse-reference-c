/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * bsp_hello.c - Simple Binsparse MEX function demonstration
 *
 * This is a minimal MEX function that demonstrates how to:
 * 1. Include Binsparse headers
 * 2. Use basic Binsparse types and error handling
 * 3. Return information to Matlab
 *
 * Usage in Matlab:
 *   info = bsp_hello();
 *   [version, success] = bsp_hello('version');
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <string.h>

/**
 * Dummy function that demonstrates basic Binsparse functionality
 */
bsp_error_t dummy_binsparse_function(char** version_out) {
  // Simulate some basic Binsparse operation
  // In a real function, this might read/write matrices, etc.

  if (!version_out) {
    return BSP_ERROR_INVALID_INPUT;
  }

  // Allocate memory for version string
  *version_out = (char*) malloc(strlen(BINSPARSE_VERSION) + 1);
  if (!*version_out) {
    return BSP_ERROR_MEMORY;
  }

  strcpy(*version_out, BINSPARSE_VERSION);
  return BSP_SUCCESS;
}

/**
 * Main MEX function entry point
 */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  char* mode = NULL;
  char* version_str = NULL;
  bsp_error_t error;

  // Handle different calling modes
  if (nrhs == 0) {
    // Default mode: return basic info
    plhs[0] = mxCreateString("Binsparse MEX binding is working!");
    return;
  }

  if (nrhs == 1) {
    // Get input argument
    if (!mxIsChar(prhs[0])) {
      mexErrMsgIdAndTxt("BinSparse:InvalidInput", "Input must be a string");
    }

    mode = mxArrayToString(prhs[0]);
    if (!mode) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert input string");
    }

    if (strcmp(mode, "version") == 0) {
      // Call our dummy Binsparse function
      error = dummy_binsparse_function(&version_str);

      if (nlhs >= 1) {
        if (error == BSP_SUCCESS) {
          plhs[0] = mxCreateString(version_str);
        } else {
          plhs[0] = mxCreateString(bsp_get_error_string(error));
        }
      }

      if (nlhs >= 2) {
        plhs[1] = mxCreateLogicalScalar(error == BSP_SUCCESS);
      }

      // Clean up
      if (version_str) {
        free(version_str);
      }
    } else {
      mexErrMsgIdAndTxt("BinSparse:InvalidMode",
                        "Unknown mode. Valid modes: 'version'");
    }

    // Clean up mode string
    mxFree(mode);
  } else {
    mexErrMsgIdAndTxt("BinSparse:TooManyInputs",
                      "Too many input arguments. Expected 0 or 1.");
  }
}
