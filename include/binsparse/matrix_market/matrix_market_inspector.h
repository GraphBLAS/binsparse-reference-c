/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <binsparse/structure.h>

// Read metadata from Matrix Market file.
// Returns a tuple holding 5 values.
// 0 - number of rows in matrix
// 1 - number of columns in matrix
// 2 - number of values in matrix
// 3 - type of the matrix (real / integer / complex / pattern)
// 4 - structure of the matrix (general / symmetric / skew-symmetric /
//     Hermitian)
// 5 - comments

typedef struct bsp_mm_metadata {
  size_t nrows;
  size_t ncols;
  size_t nnz;
  char format[32];
  char type[32];
  char structure[32];
  char* comments;
} bsp_mm_metadata;

static inline void bsp_destroy_mm_metadata(bsp_mm_metadata* metadata) {
  free(metadata->comments);
}

static inline bool bsp_mm_metadata_is_valid(bsp_mm_metadata metadata) {
  return metadata.format[0] != '\0' && metadata.type[0] != '\0' &&
         metadata.structure[0] != '\0';
}

static inline bsp_mm_metadata bsp_mmread_metadata(const char* file_path) {
  bsp_mm_metadata metadata = {0};
  metadata.comments = (char*) calloc(1, sizeof(char));
  if (metadata.comments == NULL) {
    return metadata;
  }

  FILE* f = fopen(file_path, "r");

  if (f == NULL) {
    return metadata;
  }

  int read_items = fscanf(f, "%%%%MatrixMarket matrix %s %s %s\n",
                          metadata.format, metadata.type, metadata.structure);

  if (read_items != 3) {
    fclose(f);
    metadata.format[0] = '\0';
    metadata.type[0] = '\0';
    metadata.structure[0] = '\0';
    return metadata;
  }

  for (size_t i = 0; i < strlen(metadata.structure); i++) {
    metadata.structure[i] = tolower(metadata.structure[i]);
  }

  char buf[2048];
  int outOfComments = 0;

  size_t comments_capacity = 2048;
  size_t comments_size = 0;
  char* comments = (char*) malloc(sizeof(char) * comments_capacity);
  if (comments == NULL) {
    fclose(f);
    metadata.format[0] = '\0';
    metadata.type[0] = '\0';
    metadata.structure[0] = '\0';
    return metadata;
  }
  comments[0] = '\0';

  while (!outOfComments) {
    char* line = fgets(buf, 2048, f);
    if (line == NULL) {
      free(comments);
      fclose(f);
      metadata.format[0] = '\0';
      metadata.type[0] = '\0';
      metadata.structure[0] = '\0';
      return metadata;
    }

    if (line[0] != '%') {
      outOfComments = 1;
    }

    if (!outOfComments) {
      if (comments_size + strlen(line) > comments_capacity) {
        while (comments_size + strlen(line) > comments_capacity) {
          comments_capacity <<= 1;
        }
        char* resized =
            (char*) realloc(comments, sizeof(char) * comments_capacity);
        if (resized == NULL) {
          free(comments);
          fclose(f);
          metadata.format[0] = '\0';
          metadata.type[0] = '\0';
          metadata.structure[0] = '\0';
          return metadata;
        }
        comments = resized;
      }

      memcpy(comments + comments_size, line, strlen(line));
      comments_size += strlen(line);
      comments[comments_size] = '\0';
    }
  }

  if (comments_size > 0 && comments[comments_size - 1] == '\n') {
    comments[comments_size - 1] = 0;
  }

  free(metadata.comments);
  metadata.comments = comments;

  unsigned long long nrows, ncols, nnz;
  if (strcmp(metadata.format, "coordinate") == 0) {
    if (sscanf(buf, "%llu %llu %llu", &nrows, &ncols, &nnz) != 3) {
      fclose(f);
      metadata.format[0] = '\0';
      metadata.type[0] = '\0';
      metadata.structure[0] = '\0';
      return metadata;
    }
  } else {
    if (sscanf(buf, "%llu %llu", &nrows, &ncols) != 2) {
      fclose(f);
      metadata.format[0] = '\0';
      metadata.type[0] = '\0';
      metadata.structure[0] = '\0';
      return metadata;
    }
    nnz = nrows * ncols;
  }

  metadata.nrows = nrows;
  metadata.ncols = ncols;
  metadata.nnz = nnz;

  fclose(f);

  return metadata;
}
