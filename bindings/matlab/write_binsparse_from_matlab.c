/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * write_binsparse_from_matlab.c - Write a SuiteSparse Matrix Collection
 * problem struct to Binsparse format.
 *
 * This MEX entry point intentionally delegates SSMC-specific handling to the
 * MATLAB implementation in generate_bsp_from_ssmc.m.  That path writes the
 * primary matrix, explicit zeros, metadata, aux matrices, b, and x.
 *
 * Usage in MATLAB/Octave:
 *   write_binsparse_from_matlab(problem_struct, filename)
 *   write_binsparse_from_matlab(problem_struct, filename, format)
 *   write_binsparse_from_matlab(problem_struct, filename, format, [], compression)
 *
 * For compatibility with the older scaffold, a non-format third string is
 * accepted and ignored as an HDF5 group name; generate_bsp_from_ssmc writes
 * SSMC files at the root with aux entries in named groups.
 */

#include "mex.h"
#include <stdbool.h>
#include <strings.h>

static bool is_supported_sparse_format(const char* text) {
  return strcasecmp(text, "CSC") == 0 || strcasecmp(text, "CSR") == 0 ||
         strcasecmp(text, "COO") == 0 || strcasecmp(text, "COOR") == 0;
}

static mxArray* make_default_format(void) { return mxCreateString("COO"); }

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  (void) plhs;

  if (nrhs < 2 || nrhs > 5) {
    mexErrMsgIdAndTxt(
        "BinSparse:InvalidArgs",
        "Usage: write_binsparse_from_matlab(problem_struct, filename [, format "
        "[, json_metadata [, compression_level]]])");
  }

  if (nlhs > 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "No output arguments expected");
  }

  if (!mxIsStruct(prhs[0])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidProblem",
                      "First argument must be a struct");
  }

  if (!mxIsChar(prhs[1])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFilename",
                      "Filename must be a character vector");
  }

  mxArray* format = NULL;
  if (nrhs >= 3 && !mxIsEmpty(prhs[2])) {
    if (!mxIsChar(prhs[2])) {
      mexErrMsgIdAndTxt("BinSparse:InvalidFormat",
                        "Format must be a character vector");
    }

    char* format_text = mxArrayToString(prhs[2]);
    if (!format_text) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to read format string");
    }

    if (is_supported_sparse_format(format_text)) {
      format = mxCreateString(format_text);
    }
    mxFree(format_text);
  }

  if (!format) {
    format = make_default_format();
  }

  mxArray* compression = NULL;
  if (nrhs >= 5 && !mxIsEmpty(prhs[4])) {
    if (!mxIsNumeric(prhs[4]) || mxIsComplex(prhs[4]) ||
        mxGetNumberOfElements(prhs[4]) != 1) {
      mxDestroyArray(format);
      mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                        "Compression level must be a real numeric scalar");
    }
    compression = mxDuplicateArray(prhs[4]);
  } else if (nrhs >= 4 && mxIsNumeric(prhs[3]) && !mxIsEmpty(prhs[3])) {
    compression = mxDuplicateArray(prhs[3]);
  } else {
    compression = mxCreateDoubleScalar(0.0);
  }

  mxArray* rhs[4];
  rhs[0] = (mxArray*) prhs[0];
  rhs[1] = (mxArray*) prhs[1];
  rhs[2] = format;
  rhs[3] = compression;

  int status = mexCallMATLAB(0, NULL, 4, rhs, "generate_bsp_from_ssmc");

  mxDestroyArray(format);
  mxDestroyArray(compression);

  if (status != 0) {
    mexErrMsgIdAndTxt("BinSparse:WriteFailed",
                      "generate_bsp_from_ssmc failed");
  }
}
