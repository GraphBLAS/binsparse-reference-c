/*
 * SPDX-FileCopyrightText: 2026 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * matlab_bsp_strings.h - shared helpers for the HDF5 text datasets that carry
 * SuiteSparse Matrix Collection `aux` text components.
 *
 * Wire format
 * -----------
 * A text component is a one-dimensional HDF5 string dataset with character set
 * H5T_CSET_UTF8.  The string datatype records which MATLAB class the component
 * came from, so the file is self-describing and no side-channel metadata is
 * needed to read it back:
 *
 *   fixed-length (H5T_STR_NULLPAD)  <->  MATLAB char matrix, m-by-n
 *   variable-length (H5T_VARIABLE)  <->  MATLAB cellstr, m-by-1
 *
 * A char matrix is rectangular, so every element of a fixed-length dataset
 * holds one complete row: all n characters, including the trailing blanks
 * MATLAB uses to pad short rows.  The datatype size W is the largest UTF-8
 * encoding of a row, in bytes; rows that encode to fewer bytes are padded with
 * NULs.  For the ASCII text that makes up nearly all of the collection W is
 * exactly n and no padding is stored at all, so the width is carried by the
 * datatype and the round trip is exact without deblanking either side.
 *
 * NULs pad rather than blanks because HDF5 sizes a fixed-length string in
 * bytes while MATLAB sizes a char matrix in characters.  The two agree only
 * for ASCII; blank padding would silently widen any row holding a multi-byte
 * character.  Stripping trailing NULs recovers the row's exact n characters
 * whatever it contains.  Trailing blanks are stripped as well when a foreign
 * file declares H5T_STR_SPACEPAD, which is the Fortran convention.
 *
 * A cellstr is a ragged list rather than a rectangle, so it is stored
 * variable-length and needs no padding convention.
 */

#ifndef MATLAB_BSP_STRINGS_H
#define MATLAB_BSP_STRINGS_H

#include "mex.h"
#include <hdf5.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BSP_UTF_INVALID ((size_t) -1)

/* Chunks are sized to about this many bytes when a text dataset is compressed.
 */
#define BSP_TEXT_CHUNK_BYTES ((size_t) (1024 * 1024))

/* Below this many bytes the chunk index costs more than the filter saves. */
#define BSP_TEXT_MIN_COMPRESSED_BYTES ((size_t) 4096)

/*----------------------------------------------------------------------------
 * UTF-16 (the encoding of a MATLAB mxChar) <-> UTF-8
 *--------------------------------------------------------------------------*/

// Number of UTF-8 bytes needed for len UTF-16 code units, or BSP_UTF_INVALID
// if the sequence contains an unpaired surrogate.
static inline size_t bsp_utf8_length(const mxChar* src, size_t len) {
  size_t bytes = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t unit = src[i];
    if (unit < 0x80u) {
      bytes += 1;
    } else if (unit < 0x800u) {
      bytes += 2;
    } else if (unit >= 0xD800u && unit <= 0xDBFFu) {
      if (i + 1 >= len || src[i + 1] < 0xDC00u || src[i + 1] > 0xDFFFu) {
        return BSP_UTF_INVALID;
      }
      bytes += 4;
      i++;
    } else if (unit >= 0xDC00u && unit <= 0xDFFFu) {
      return BSP_UTF_INVALID;
    } else {
      bytes += 3;
    }
  }
  return bytes;
}

// Encode len UTF-16 code units into out, which must hold bsp_utf8_length
// bytes.  Returns the number of bytes written.  The caller is responsible for
// having rejected unpaired surrogates via bsp_utf8_length.
static inline size_t bsp_utf8_encode(const mxChar* src, size_t len, char* out) {
  unsigned char* p = (unsigned char*) out;
  for (size_t i = 0; i < len; i++) {
    uint32_t code = src[i];
    if (code >= 0xD800u && code <= 0xDBFFu && i + 1 < len &&
        src[i + 1] >= 0xDC00u && src[i + 1] <= 0xDFFFu) {
      code = 0x10000u + ((code - 0xD800u) << 10) + (src[i + 1] - 0xDC00u);
      i++;
    }
    if (code < 0x80u) {
      *p++ = (unsigned char) code;
    } else if (code < 0x800u) {
      *p++ = (unsigned char) (0xC0u | (code >> 6));
      *p++ = (unsigned char) (0x80u | (code & 0x3Fu));
    } else if (code < 0x10000u) {
      *p++ = (unsigned char) (0xE0u | (code >> 12));
      *p++ = (unsigned char) (0x80u | ((code >> 6) & 0x3Fu));
      *p++ = (unsigned char) (0x80u | (code & 0x3Fu));
    } else {
      *p++ = (unsigned char) (0xF0u | (code >> 18));
      *p++ = (unsigned char) (0x80u | ((code >> 12) & 0x3Fu));
      *p++ = (unsigned char) (0x80u | ((code >> 6) & 0x3Fu));
      *p++ = (unsigned char) (0x80u | (code & 0x3Fu));
    }
  }
  return (size_t) ((char*) p - out);
}

// Number of UTF-16 code units needed for len UTF-8 bytes, or BSP_UTF_INVALID
// if the bytes are not well-formed UTF-8.
static inline size_t bsp_utf16_length(const char* src, size_t len) {
  const unsigned char* p = (const unsigned char*) src;
  size_t units = 0;
  size_t i = 0;
  while (i < len) {
    unsigned char lead = p[i];
    size_t trail;
    uint32_t code;
    if (lead < 0x80u) {
      trail = 0;
      code = lead;
    } else if ((lead & 0xE0u) == 0xC0u) {
      trail = 1;
      code = lead & 0x1Fu;
    } else if ((lead & 0xF0u) == 0xE0u) {
      trail = 2;
      code = lead & 0x0Fu;
    } else if ((lead & 0xF8u) == 0xF0u) {
      trail = 3;
      code = lead & 0x07u;
    } else {
      return BSP_UTF_INVALID;
    }
    if (i + trail >= len) {
      return BSP_UTF_INVALID;
    }
    for (size_t k = 1; k <= trail; k++) {
      if ((p[i + k] & 0xC0u) != 0x80u) {
        return BSP_UTF_INVALID;
      }
      code = (code << 6) | (uint32_t) (p[i + k] & 0x3Fu);
    }
    if (code > 0x10FFFFu) {
      return BSP_UTF_INVALID;
    }
    units += (code >= 0x10000u) ? 2 : 1;
    i += trail + 1;
  }
  return units;
}

// Decode len UTF-8 bytes into out, which must hold bsp_utf16_length code
// units.  Returns the number of code units written.  The caller is responsible
// for having validated the bytes via bsp_utf16_length.
static inline size_t bsp_utf16_decode(const char* src, size_t len,
                                      mxChar* out) {
  const unsigned char* p = (const unsigned char*) src;
  size_t units = 0;
  size_t i = 0;
  while (i < len) {
    unsigned char lead = p[i];
    size_t trail;
    uint32_t code;
    if (lead < 0x80u) {
      trail = 0;
      code = lead;
    } else if ((lead & 0xE0u) == 0xC0u) {
      trail = 1;
      code = lead & 0x1Fu;
    } else if ((lead & 0xF0u) == 0xE0u) {
      trail = 2;
      code = lead & 0x0Fu;
    } else {
      trail = 3;
      code = lead & 0x07u;
    }
    for (size_t k = 1; k <= trail; k++) {
      code = (code << 6) | (uint32_t) (p[i + k] & 0x3Fu);
    }
    if (code >= 0x10000u) {
      code -= 0x10000u;
      out[units++] = (mxChar) (0xD800u + (code >> 10));
      out[units++] = (mxChar) (0xDC00u + (code & 0x3FFu));
    } else {
      out[units++] = (mxChar) code;
    }
    i += trail + 1;
  }
  return units;
}

// Convert a MATLAB char array to a NUL-terminated UTF-8 string allocated with
// mxMalloc.  Returns NULL if the array is not char, holds an unpaired
// surrogate, or contains a NUL, which a C string cannot carry.  Unlike
// mxArrayToString this preserves every character: mxArrayToString goes through
// the local code page and replaces anything it cannot represent with SUB
// (0x1A), which silently corrupts non-ASCII text.
static inline char* bsp_mx_to_utf8(const mxArray* value) {
  if (!value || !mxIsChar(value)) {
    return NULL;
  }

  size_t len = mxGetNumberOfElements(value);
  const mxChar* chars = (const mxChar*) mxGetData(value);
  for (size_t i = 0; i < len; i++) {
    if (chars[i] == 0) {
      return NULL;
    }
  }

  size_t bytes = bsp_utf8_length(chars, len);
  if (bytes == BSP_UTF_INVALID) {
    return NULL;
  }

  char* encoded = (char*) mxMalloc(bytes + 1);
  bsp_utf8_encode(chars, len, encoded);
  encoded[bytes] = '\0';
  return encoded;
}

/*----------------------------------------------------------------------------
 * HDF5 datatype and property list helpers
 *--------------------------------------------------------------------------*/

// Create the fixed-length, NUL-padded UTF-8 datatype used for char matrices.
static inline hid_t bsp_fixed_string_type(size_t width) {
  hid_t type = H5Tcopy(H5T_C_S1);
  if (type == H5I_INVALID_HID) {
    return H5I_INVALID_HID;
  }
  if (H5Tset_size(type, width) < 0 ||
      H5Tset_strpad(type, H5T_STR_NULLPAD) < 0 ||
      H5Tset_cset(type, H5T_CSET_UTF8) < 0) {
    H5Tclose(type);
    return H5I_INVALID_HID;
  }
  return type;
}

// Create the variable-length UTF-8 datatype used for cellstr.
static inline hid_t bsp_variable_string_type(void) {
  hid_t type = H5Tcopy(H5T_C_S1);
  if (type == H5I_INVALID_HID) {
    return H5I_INVALID_HID;
  }
  if (H5Tset_size(type, H5T_VARIABLE) < 0 ||
      H5Tset_cset(type, H5T_CSET_UTF8) < 0) {
    H5Tclose(type);
    return H5I_INVALID_HID;
  }
  return type;
}

// Build a dataset creation property list that chunks and deflates a text
// dataset of count elements of element_size bytes.  Returns H5P_DEFAULT when
// compression is not requested or when the dataset is too small for a chunk
// index to pay for itself.  The threshold is on the total size rather than on
// the element count, so that one very wide row is still compressed.
//
// A fixed-length dataset compresses well: blank padding is highly redundant,
// and SNAP/wiki-topcats pagenames goes from 372.6 MB to 19.2 MB.  A
// variable-length one barely moves, because HDF5 keeps the strings themselves
// on the global heap, which the filter pipeline does not reach; only the
// 16-byte-per-element descriptor array is filtered.  Requesting compression is
// still worthwhile there, since that descriptor array grows with the element
// count, but the payload cannot be reached from here.
static inline hid_t bsp_text_dataset_properties(size_t count,
                                                size_t element_size,
                                                int compression_level) {
  if (compression_level <= 0 || element_size == 0 ||
      count * element_size < BSP_TEXT_MIN_COMPRESSED_BYTES) {
    return H5P_DEFAULT;
  }

  size_t chunk = BSP_TEXT_CHUNK_BYTES / element_size;
  if (chunk == 0) {
    chunk = 1;
  }
  if (chunk > count) {
    chunk = count;
  }

  hid_t properties = H5Pcreate(H5P_DATASET_CREATE);
  if (properties == H5I_INVALID_HID) {
    return H5P_DEFAULT;
  }

  hsize_t chunk_dims[1] = {(hsize_t) chunk};
  if (H5Pset_chunk(properties, 1, chunk_dims) < 0 ||
      H5Pset_deflate(properties, (unsigned) compression_level) < 0) {
    H5Pclose(properties);
    return H5P_DEFAULT;
  }
  return properties;
}

static inline void bsp_close_text_dataset_properties(hid_t properties) {
  if (properties != H5P_DEFAULT && properties != H5I_INVALID_HID) {
    H5Pclose(properties);
  }
}

// Keep a MEX function loaded for the whole MATLAB session, and stop HDF5 from
// registering atexit handlers: both guard against crashes when MATLAB tears
// down MEX files that share HDF5 process-wide state.
static inline void bsp_lock_mex_module(void) {
  if (!mexIsLocked()) {
    H5dont_atexit();
    mexLock();
  }
}

#endif // MATLAB_BSP_STRINGS_H
