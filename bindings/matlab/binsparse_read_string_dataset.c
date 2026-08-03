/*
 * SPDX-FileCopyrightText: 2026 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_read_string_dataset.c - Read an HDF5 UTF-8 string dataset.
 *
 * Usage in MATLAB/Octave:
 *   value = binsparse_read_string_dataset(filename, dataset_name)
 *
 * The MATLAB class of the result comes from the HDF5 string datatype: a
 * fixed-length dataset reads back as a char matrix and a variable-length one as
 * an m-by-1 cellstr.  See matlab_bsp_strings.h for the format.
 */

#include "matlab_bsp_strings.h"
#include "mex.h"
#include <hdf5.h>
#include <stdbool.h>
#include <string.h>

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

// Number of elements in a scalar or one-dimensional dataspace.  Returns
// (hssize_t) -1 for any other shape.
static hssize_t string_dataset_count(hid_t space) {
  int rank = H5Sget_simple_extent_ndims(space);
  if (rank < 0 || rank > 1) {
    return -1;
  }
  if (rank == 0) {
    // Text written before the fixed/variable convention was introduced could
    // be a scalar dataset holding a single string.
    return 1;
  }
  hsize_t dims[1] = {0};
  if (H5Sget_simple_extent_dims(space, dims, NULL) < 0) {
    return -1;
  }
  return (hssize_t) dims[0];
}

/*----------------------------------------------------------------------------
 * fixed-length dataset -> char matrix
 *--------------------------------------------------------------------------*/

static mxArray* read_fixed_strings(hid_t dset, hid_t ftype, size_t count,
                                   const char** error_id,
                                   const char** error_message) {
  size_t width = H5Tget_size(ftype);
  if (width == 0) {
    *error_id = "BinSparse:HDF5Error";
    *error_message = "Failed to size the fixed-length string datatype";
    return NULL;
  }

  // Padding is stripped from the right: NULs always, because that is what this
  // writer pads with, and blanks as well when the file declares the Fortran
  // convention.
  bool space_padded = (H5Tget_strpad(ftype) == H5T_STR_SPACEPAD);

  char* buffer = (char*) mxCalloc(count > 0 ? count * width : 1, sizeof(char));
  if (count > 0 &&
      H5Dread(dset, ftype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer) < 0) {
    mxFree(buffer);
    *error_id = "BinSparse:HDF5Error";
    *error_message = "Failed to read the fixed-length string dataset";
    return NULL;
  }

  size_t* lengths = (size_t*) mxCalloc(count > 0 ? count : 1, sizeof(size_t));
  size_t* units = (size_t*) mxCalloc(count > 0 ? count : 1, sizeof(size_t));
  size_t columns = 0;

  for (size_t i = 0; i < count; i++) {
    const char* row = buffer + i * width;
    size_t len = width;
    while (len > 0 && (row[len - 1] == '\0' ||
                       (space_padded && row[len - 1] == ' '))) {
      len--;
    }
    size_t n = bsp_utf16_length(row, len);
    if (n == BSP_UTF_INVALID) {
      mxFree(units);
      mxFree(lengths);
      mxFree(buffer);
      *error_id = "BinSparse:InvalidText";
      *error_message = "String dataset contains malformed UTF-8";
      return NULL;
    }
    lengths[i] = len;
    units[i] = n;
    if (n > columns) {
      columns = n;
    }
  }

  // Rows written by this binding all decode to the same width.  A foreign file
  // need not be rectangular, so short rows are blank filled the way MATLAB's
  // char() would pad them.
  mwSize dims[2] = {(mwSize) count, (mwSize) columns};
  mxArray* result = mxCreateCharArray(2, dims);
  mxChar* chars = (mxChar*) mxGetData(result);
  for (size_t i = 0; i < count * columns; i++) {
    chars[i] = (mxChar) ' ';
  }

  mxChar* row_units =
      (mxChar*) mxCalloc(columns > 0 ? columns : 1, sizeof(mxChar));
  for (size_t i = 0; i < count; i++) {
    bsp_utf16_decode(buffer + i * width, lengths[i], row_units);
    for (size_t j = 0; j < units[i]; j++) {
      chars[i + j * count] = row_units[j];
    }
  }

  mxFree(row_units);
  mxFree(units);
  mxFree(lengths);
  mxFree(buffer);
  return result;
}

/*----------------------------------------------------------------------------
 * variable-length dataset -> cellstr
 *--------------------------------------------------------------------------*/

static mxArray* read_variable_strings(hid_t dset, hid_t ftype, hid_t space,
                                      size_t count, const char** error_id,
                                      const char** error_message) {
  char** buffer = (char**) mxCalloc(count > 0 ? count : 1, sizeof(char*));
  if (count > 0 &&
      H5Dread(dset, ftype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer) < 0) {
    mxFree(buffer);
    *error_id = "BinSparse:HDF5Error";
    *error_message = "Failed to read the variable-length string dataset";
    return NULL;
  }

  mxArray* result = mxCreateCellMatrix((mwSize) count, 1);
  bool malformed = false;

  for (size_t i = 0; i < count; i++) {
    const char* string = buffer[i] ? buffer[i] : "";
    size_t len = strlen(string);
    size_t n = bsp_utf16_length(string, len);
    if (n == BSP_UTF_INVALID) {
      malformed = true;
      n = 0;
      len = 0;
    }
    mwSize dims[2] = {1, (mwSize) n};
    mxArray* cell = mxCreateCharArray(2, dims);
    bsp_utf16_decode(string, len, (mxChar*) mxGetData(cell));
    mxSetCell(result, (mwIndex) i, cell);
  }

  H5Dvlen_reclaim(ftype, space, H5P_DEFAULT, buffer);
  mxFree(buffer);

  if (malformed) {
    mxDestroyArray(result);
    *error_id = "BinSparse:InvalidText";
    *error_message = "String dataset contains malformed UTF-8";
    return NULL;
  }
  return result;
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  bsp_lock_mex_module();

  if (nrhs != 2) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: value = binsparse_read_string_dataset(filename, "
                      "dataset_name)");
  }
  if (nlhs > 1) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "Only one output argument is produced");
  }

  char* filename = get_required_string(prhs[0], "filename");
  char* dataset_name = get_required_string(prhs[1], "dataset_name");

  hid_t file = H5I_INVALID_HID;
  hid_t dset = H5I_INVALID_HID;
  hid_t ftype = H5I_INVALID_HID;
  hid_t space = H5I_INVALID_HID;
  mxArray* result = NULL;
  const char* error_id = NULL;
  const char* error_message = NULL;

  file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file == H5I_INVALID_HID) {
    error_id = "BinSparse:FileError";
    error_message = "Failed to open HDF5 file";
    goto cleanup;
  }

  dset = H5Dopen2(file, dataset_name, H5P_DEFAULT);
  if (dset == H5I_INVALID_HID) {
    error_id = "BinSparse:MissingDataset";
    error_message = "Failed to open string dataset";
    goto cleanup;
  }

  ftype = H5Dget_type(dset);
  if (ftype == H5I_INVALID_HID || H5Tget_class(ftype) != H5T_STRING) {
    error_id = "BinSparse:InvalidDataset";
    error_message = "Dataset does not hold strings";
    goto cleanup;
  }

  space = H5Dget_space(dset);
  hssize_t count = (space == H5I_INVALID_HID) ? -1 : string_dataset_count(space);
  if (count < 0) {
    error_id = "BinSparse:InvalidDataset";
    error_message = "String dataset must be scalar or one-dimensional";
    goto cleanup;
  }

  htri_t variable = H5Tis_variable_str(ftype);
  if (variable < 0) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to classify the string datatype";
    goto cleanup;
  }

  // The datatype is what says which MATLAB class the text came from.
  if (variable > 0) {
    result = read_variable_strings(dset, ftype, space, (size_t) count,
                                   &error_id, &error_message);
  } else {
    result = read_fixed_strings(dset, ftype, (size_t) count, &error_id,
                                &error_message);
  }

cleanup:
  if (space != H5I_INVALID_HID) {
    H5Sclose(space);
  }
  if (ftype != H5I_INVALID_HID) {
    H5Tclose(ftype);
  }
  if (dset != H5I_INVALID_HID) {
    H5Dclose(dset);
  }
  if (file != H5I_INVALID_HID) {
    H5Fclose(file);
  }
  mxFree(dataset_name);
  mxFree(filename);

  if (error_id) {
    if (result) {
      mxDestroyArray(result);
    }
    mexErrMsgIdAndTxt(error_id, "%s", error_message);
  }

  plhs[0] = result;
}
