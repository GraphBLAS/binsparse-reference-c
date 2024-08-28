#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

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

bsp_mm_metadata bsp_mmread_metadata(char* file_path) {
  FILE* f = fopen(file_path, "r");

  assert(f != NULL);

  bsp_mm_metadata metadata;

  int read_items = fscanf(f, "%%%%MatrixMarket matrix %s %s %s\n",
                          metadata.format, metadata.type, metadata.structure);

  assert(read_items == 3);

  for (size_t i = 0; i < strlen(metadata.structure); i++) {
    metadata.structure[i] = tolower(metadata.structure[i]);
  }

  char buf[2048];
  int outOfComments = 0;

  size_t comments_capacity = 2048;
  size_t comments_size = 0;
  char* comments = (char*) malloc(sizeof(char) * comments_capacity);

  while (!outOfComments) {
    char* line = fgets(buf, 2048, f);
    assert(line != NULL);

    if (line[0] != '%') {
      outOfComments = 1;
    }

    if (!outOfComments) {
      if (comments_size + strlen(line) > comments_capacity) {
        while (comments_size + strlen(line) > comments_capacity) {
          comments_capacity <<= 1;
        }
        comments = (char*) realloc(comments, sizeof(char) * comments_capacity);
      }

      memcpy(comments + comments_size, line, strlen(line));
      comments_size += strlen(line);
    }
  }

  if (comments[comments_size - 1] == '\n') {
    comments[comments_size - 1] = 0;
  }

  metadata.comments = comments;

  unsigned long long nrows, ncols, nnz;
  if (strcmp(metadata.format, "coordinate") == 0) {
    sscanf(buf, "%llu %llu %llu", &nrows, &ncols, &nnz);
  } else {
    sscanf(buf, "%llu %llu", &nrows, &ncols);
    nnz = nrows * ncols;
  }

  metadata.nrows = nrows;
  metadata.ncols = ncols;
  metadata.nnz = nnz;

  fclose(f);

  return metadata;
}
