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
 *   binsparse_write_string_dataset(filename, dataset_name, value, level)
 *
 * A char matrix is written as a fixed-length dataset and a cell array of
 * character vectors as a variable-length one, so the datatype alone says which
 * MATLAB class the text came from.  See matlab_bsp_strings.h for the format.
 */

#include "matlab_bsp_strings.h"
#include "mex.h"
#include <hdf5.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

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

static int get_compression_level(int nrhs, const mxArray* prhs[]) {
  if (nrhs < 4 || mxIsEmpty(prhs[3])) {
    return 0;
  }
  if (!mxIsNumeric(prhs[3]) || mxIsComplex(prhs[3]) ||
      mxGetNumberOfElements(prhs[3]) != 1) {
    mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                      "Compression level must be a real numeric scalar");
  }
  double level = mxGetScalar(prhs[3]);
  if (!(level >= 0.0) || level > 9.0 || level != (double) (int) level) {
    mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                      "Compression level must be an integer from 0 to 9");
  }
  return (int) level;
}

/*----------------------------------------------------------------------------
 * char matrix -> fixed-length dataset
 *
 * Every row is encoded in full, trailing blanks included: the datatype size is
 * what carries the column count back to the reader, so nothing is deblanked.
 *--------------------------------------------------------------------------*/

static char* encode_char_matrix(const mxArray* value, size_t* rows,
                                size_t* width) {
  size_t m = mxGetM(value);
  size_t n = mxGetN(value);
  const mxChar* chars = (const mxChar*) mxGetData(value);

  // Lay each row out contiguously so it can be encoded as one string.
  mxChar* row = (mxChar*) mxCalloc(n > 0 ? n : 1, sizeof(mxChar));
  size_t max_bytes = 0;

  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
      row[j] = chars[i + j * m];
    }
    size_t bytes = bsp_utf8_length(row, n);
    if (bytes == BSP_UTF_INVALID) {
      mxFree(row);
      mexErrMsgIdAndTxt("BinSparse:InvalidText",
                        "Row %zu contains an unpaired UTF-16 surrogate", i + 1);
    }
    if (bytes > max_bytes) {
      max_bytes = bytes;
    }
  }

  // H5Tset_size rejects a zero-byte string, so an all-empty char matrix is
  // stored as one NUL per row, which strips back to zero characters.
  size_t w = max_bytes > 0 ? max_bytes : 1;
  char* buffer = (char*) mxCalloc(m > 0 ? m * w : 1, sizeof(char));

  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
      row[j] = chars[i + j * m];
    }
    // Anything past the encoded row is already NUL from mxCalloc.
    bsp_utf8_encode(row, n, buffer + i * w);
  }

  mxFree(row);

  *rows = m;
  *width = w;
  return buffer;
}

/*----------------------------------------------------------------------------
 * cellstr -> variable-length dataset
 *--------------------------------------------------------------------------*/

static char* encode_char_vector(const mxArray* value, size_t index) {
  if (!value || !mxIsChar(value)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                      "Cell element %zu is not a character vector", index + 1);
  }
  if (mxGetNumberOfDimensions(value) > 2 || mxGetM(value) > 1) {
    mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                      "Cell element %zu must be a character row vector",
                      index + 1);
  }

  size_t len = mxGetNumberOfElements(value);
  const mxChar* chars = (const mxChar*) mxGetData(value);
  for (size_t k = 0; k < len; k++) {
    if (chars[k] == 0) {
      mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                        "Cell element %zu contains a NUL character, which a "
                        "variable-length HDF5 string cannot represent",
                        index + 1);
    }
  }

  size_t bytes = bsp_utf8_length(chars, len);
  if (bytes == BSP_UTF_INVALID) {
    mexErrMsgIdAndTxt("BinSparse:InvalidText",
                      "Cell element %zu contains an unpaired UTF-16 surrogate",
                      index + 1);
  }

  char* encoded = (char*) mxCalloc(bytes + 1, sizeof(char));
  bsp_utf8_encode(chars, len, encoded);
  encoded[bytes] = '\0';
  return encoded;
}

static char** encode_cellstr(const mxArray* value, size_t* count) {
  size_t m = mxGetNumberOfElements(value);
  char** strings = (char**) mxCalloc(m > 0 ? m : 1, sizeof(char*));
  for (size_t i = 0; i < m; i++) {
    strings[i] = encode_char_vector(mxGetCell(value, i), i);
  }
  *count = m;
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
  bsp_lock_mex_module();

  if (nrhs < 3 || nrhs > 4) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: binsparse_write_string_dataset(filename, "
                      "dataset_name, value [, compression_level])");
  }

  if (nlhs > 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "No output arguments expected");
  }

  char* filename = get_required_string(prhs[0], "filename");
  char* dataset_name = get_required_string(prhs[1], "dataset_name");
  int compression_level = get_compression_level(nrhs, prhs);

  const mxArray* value = prhs[2];
  bool is_cellstr = mxIsCell(value);
  if ((!is_cellstr && !mxIsChar(value)) || mxGetNumberOfDimensions(value) > 2) {
    mxFree(dataset_name);
    mxFree(filename);
    mexErrMsgIdAndTxt("BinSparse:InvalidValue",
                      "Value must be a two-dimensional char matrix or a cell "
                      "array of character vectors");
  }

  size_t count = 0;
  size_t width = 0;
  char* fixed_buffer = NULL;
  char** strings = NULL;
  if (is_cellstr) {
    strings = encode_cellstr(value, &count);
  } else {
    fixed_buffer = encode_char_matrix(value, &count, &width);
  }

  hid_t file = H5I_INVALID_HID;
  hid_t type = H5I_INVALID_HID;
  hid_t space = H5I_INVALID_HID;
  hid_t properties = H5P_DEFAULT;
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

  type = is_cellstr ? bsp_variable_string_type() : bsp_fixed_string_type(width);
  if (type == H5I_INVALID_HID) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create UTF-8 string datatype";
    goto cleanup;
  }

  hsize_t dims[1] = {(hsize_t) count};
  space = H5Screate_simple(1, dims, NULL);
  if (space == H5I_INVALID_HID) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create string dataspace";
    goto cleanup;
  }

  properties = bsp_text_dataset_properties(
      count, is_cellstr ? sizeof(char*) : width, compression_level);

  dset = H5Dcreate2(file, dataset_name, type, space, H5P_DEFAULT, properties,
                    H5P_DEFAULT);
  if (dset == H5I_INVALID_HID) {
    error_id = "BinSparse:HDF5Error";
    error_message = "Failed to create string dataset";
    goto cleanup;
  }

  if (count > 0) {
    const void* data =
        is_cellstr ? (const void*) strings : (const void*) fixed_buffer;
    if (H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0) {
      error_id = "BinSparse:HDF5Error";
      error_message = "Failed to write string dataset";
      goto cleanup;
    }
  }

cleanup:
  if (dset != H5I_INVALID_HID) {
    H5Dclose(dset);
  }
  bsp_close_text_dataset_properties(properties);
  if (space != H5I_INVALID_HID) {
    H5Sclose(space);
  }
  if (type != H5I_INVALID_HID) {
    H5Tclose(type);
  }
  if (file != H5I_INVALID_HID) {
    H5Fclose(file);
  }
  if (fixed_buffer) {
    mxFree(fixed_buffer);
  }
  free_strings(strings, count);
  mxFree(dataset_name);
  mxFree(filename);

  if (error_id) {
    mexErrMsgIdAndTxt(error_id, "%s", error_message);
  }
}
