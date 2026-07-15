/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_write_string_dataset.c - Write an HDF5 UTF-8 string dataset.
 *
 * Usage in MATLAB/Octave:
 *   binsparse_write_string_dataset(filename, dataset_name, value)
 *
 * The value may be a character vector or a cell array of character vectors.
 * Strings are stored as variable-length UTF-8 HDF5 strings, matching the usual
 * h5py representation for Python str values.
 */

#include "mex.h"
#include <hdf5.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static void lock_mex_module(void) {
  static bool locked = false;
  if (!locked) {
    H5dont_atexit();
    mexLock();
    locked = true;
  }
}

static char* get_required_string(const mxArray* value, const char* name) {
  if (!mxIsChar(value)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidString",
                      "%s must be a character vector", name);
  }

  char* string = mxArrayToString(value);
  if (!string) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError", "Failed to read %s", name);
  }
  return string;
}

static char** collect_strings(const mxArray* value, size_t* count,
                              bool* scalar_dataset) {
  char** strings = NULL;

  if (mxIsCell(value)) {
    *count = mxGetNumberOfElements(value);
    *scalar_dataset = false;
    strings = (char**) mxCalloc(*count, sizeof(char*));
    for (size_t i = 0; i < *count; i++) {
      const mxArray* cell = mxGetCell(value, i);
      if (!cell || !mxIsChar(cell)) {
        mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                          "Cell values must be character vectors");
      }
      strings[i] = mxArrayToString(cell);
      if (!strings[i]) {
        mexErrMsgIdAndTxt("BinSparse:MemoryError",
                          "Failed to read string cell");
      }
    }
  } else if (mxIsChar(value)) {
    *count = 1;
    *scalar_dataset = true;
    strings = (char**) mxCalloc(1, sizeof(char*));
    strings[0] = mxArrayToString(value);
    if (!strings[0]) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError", "Failed to read string");
    }
  } else {
    mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                      "Value must be a character vector or cellstr");
  }

  return strings;
}

static void free_strings(char** strings, size_t count) {
  if (!strings) {
    return;
  }
  for (size_t i = 0; i < count; i++) {
    if (strings[i]) {
      mxFree(strings[i]);
    }
  }
  mxFree(strings);
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  (void) plhs;
  lock_mex_module();

  if (nrhs != 3) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: binsparse_write_string_dataset(filename, "
                      "dataset_name, value)");
  }

  if (nlhs > 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "No output arguments expected");
  }

  char* filename = get_required_string(prhs[0], "filename");
  char* dataset_name = get_required_string(prhs[1], "dataset_name");

  size_t count = 0;
  bool scalar_dataset = false;
  char** strings = collect_strings(prhs[2], &count, &scalar_dataset);

  hid_t file = H5I_INVALID_HID;
  hid_t type = H5I_INVALID_HID;
  hid_t space = H5I_INVALID_HID;
  hid_t dset = H5I_INVALID_HID;
  const char* error_id = NULL;
  const char* error_message = NULL;

  if (access(filename, F_OK) == 0) {
    file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  } else {
    file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  if (file == H5I_INVALID_HID) {
    error_id = "BinSparse:FileError";
    error_message = "Failed to open HDF5 file";
    goto cleanup;
  }

  htri_t exists = H5Lexists(file, dataset_name, H5P_DEFAULT);
  if (exists > 0) {
    error_id = "BinSparse:DatasetExists";
    error_message = "String dataset name already exists";
    goto cleanup;
  } else if (exists < 0) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to check string dataset name";
    goto cleanup;
  }

  type = H5Tcopy(H5T_C_S1);
  if (type == H5I_INVALID_HID ||
      H5Tset_size(type, H5T_VARIABLE) < 0 ||
      H5Tset_cset(type, H5T_CSET_UTF8) < 0) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create UTF-8 string datatype";
    goto cleanup;
  }

  if (scalar_dataset) {
    space = H5Screate(H5S_SCALAR);
  } else {
    hsize_t dims[1] = {(hsize_t) count};
    space = H5Screate_simple(1, dims, NULL);
  }
  if (space == H5I_INVALID_HID) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create string dataspace";
    goto cleanup;
  }

  dset = H5Dcreate2(file, dataset_name, type, space, H5P_DEFAULT,
                    H5P_DEFAULT, H5P_DEFAULT);
  if (dset == H5I_INVALID_HID) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create string dataset";
    goto cleanup;
  }

  if (H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, strings) < 0) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to write string dataset";
    goto cleanup;
  }

cleanup:
  if (dset != H5I_INVALID_HID) {
    H5Dclose(dset);
  }
  if (space != H5I_INVALID_HID) {
    H5Sclose(space);
  }
  if (type != H5I_INVALID_HID) {
    H5Tclose(type);
  }
  if (file != H5I_INVALID_HID) {
    H5Fclose(file);
  }
  free_strings(strings, count);
  mxFree(dataset_name);
  mxFree(filename);

  if (error_id) {
    mexErrMsgIdAndTxt(error_id, "%s", error_message);
  }
}
