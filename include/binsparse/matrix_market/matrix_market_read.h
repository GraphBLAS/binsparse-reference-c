#pragma once

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include <binsparse/matrix_market/coo_sort_tools.h>
#include <binsparse/matrix_market/matrix_market_inspector.h>
#include <binsparse/matrix_market/matrix_market_type_t.h>

bsp_matrix_t bsp_mmread_explicit(char* file_path, bsp_type_t value_type,
                                 bsp_type_t index_type) {
  bsp_mm_metadata metadata = bsp_mmread_metadata(file_path);

  assert(strcmp(metadata.format, "coordinate") == 0);

  bsp_matrix_market_type_t mm_type;
  if (strcmp(metadata.type, "pattern") == 0) {
    mm_type = BSP_MM_PATTERN;
  } else if (strcmp(metadata.type, "real") == 0) {
    mm_type = BSP_MM_REAL;
  } else if (strcmp(metadata.type, "integer") == 0) {
    mm_type = BSP_MM_INTEGER;
  } else if (strcmp(metadata.type, "complex") == 0) {
    // Don't handle complex yet.
    mm_type = BSP_MM_COMPLEX;
    assert(false);
  }

  bsp_matrix_t matrix = bsp_construct_default_matrix_t();

  matrix.nrows = metadata.nrows;
  matrix.ncols = metadata.ncols;
  matrix.nnz = metadata.nnz;

  matrix.indices_0 = bsp_construct_array_t(matrix.nnz, index_type);
  matrix.indices_1 = bsp_construct_array_t(matrix.nnz, index_type);

  if (mm_type != BSP_MM_COMPLEX) {
    matrix.values = bsp_construct_array_t(matrix.nnz, value_type);
  } else {
    matrix.values = bsp_construct_array_t(matrix.nnz * 2, value_type);
  }

  matrix.format = BSP_COO;

  int pattern = strcmp(metadata.type, "pattern") == 0;

  if (strcmp(metadata.structure, "symmetric") == 0) {
    matrix.structure = BSP_SYMMETRIC;
  } else if (strcmp(metadata.structure, "hermitian") == 0) {
    matrix.structure = BSP_HERMITIAN;
  } else if (strcmp(metadata.structure, "skew-symmetric") == 0) {
    matrix.structure = BSP_SKEW_SYMMETRIC;
  }

  FILE* f = fopen(file_path, "r");

  assert(f != NULL);

  char buf[2048];
  int outOfComments = 0;
  while (!outOfComments) {
    char* line = fgets(buf, 2048, f);
    assert(line != NULL);

    if (line[0] != '%') {
      outOfComments = 1;
    }
  }

  int eof = 0;

  size_t count = 0;
  while (fgets(buf, 2048, f) != NULL) {
    if (mm_type == BSP_MM_PATTERN) {
      unsigned long long i, j;
      sscanf(buf, "%llu %llu", &i, &j);
      i--;
      j--;

      bsp_array_write(matrix.values, count, 1);
      bsp_array_write(matrix.indices_0, count, i);
      bsp_array_write(matrix.indices_1, count, j);
    } else if (mm_type == BSP_MM_REAL) {
      unsigned long long i, j;
      double value;
      sscanf(buf, "%llu %llu %lf", &i, &j, &value);
      i--;
      j--;

      bsp_array_write(matrix.values, count, value);
      bsp_array_write(matrix.indices_0, count, i);
      bsp_array_write(matrix.indices_1, count, j);
    } else if (mm_type == BSP_MM_INTEGER) {
      unsigned long long i, j;
      long long value;
      sscanf(buf, "%llu %llu %lld", &i, &j, &value);
      i--;
      j--;

      bsp_array_write(matrix.values, count, value);
      bsp_array_write(matrix.indices_0, count, i);
      bsp_array_write(matrix.indices_1, count, j);
    } else if (mm_type == BSP_MM_COMPLEX) {
      // Don't handle complex yet.
      assert(false);
    }
    count++;
  }

  size_t* indices = malloc(sizeof(size_t) * matrix.nnz);

  for (size_t i = 0; i < matrix.nnz; i++) {
    indices[i] = i;
  }

  bsp_coo_indices_.rowind = matrix.indices_0;
  bsp_coo_indices_.colind = matrix.indices_1;

  qsort(indices, matrix.nnz, sizeof(size_t),
        bsp_coo_comparison_row_sort_operator_impl_);

  bsp_array_t rowind = bsp_copy_construct_array_t(matrix.indices_0);
  bsp_array_t colind = bsp_copy_construct_array_t(matrix.indices_1);
  bsp_array_t values = bsp_copy_construct_array_t(matrix.values);

  for (size_t i = 0; i < matrix.nnz; i++) {
    bsp_array_awrite(rowind, i, matrix.indices_0, indices[i]);
    bsp_array_awrite(colind, i, matrix.indices_1, indices[i]);
    bsp_array_awrite(values, i, matrix.values, indices[i]);
  }

  bsp_destroy_array_t(matrix.indices_0);
  bsp_destroy_array_t(matrix.indices_1);
  bsp_destroy_array_t(matrix.values);
  matrix.indices_0 = rowind;
  matrix.indices_1 = colind;
  matrix.values = values;

  free(indices);
  fclose(f);

  return matrix;
}

bsp_matrix_t bsp_mmread(char* file_path) {
  bsp_mm_metadata metadata = bsp_mmread_metadata(file_path);

  bsp_type_t value_type;

  if (strcmp(metadata.type, "real") == 0) {
    value_type = BSP_FLOAT64;
  } else if (strcmp(metadata.type, "integer") == 0) {
    value_type = BSP_INT64;
  } else if (strcmp(metadata.type, "complex") == 0) {
    value_type = BSP_COMPLEX_FLOAT64;
  } else if (strcmp(metadata.type, "pattern") == 0) {
    value_type = BSP_BINT8;
  } else {
    assert(false);
  }

  bsp_type_t index_type;

  size_t max_dim =
      (metadata.nrows > metadata.ncols) ? metadata.nrows : metadata.ncols;

  if (max_dim < (size_t)UINT8_MAX) {
    index_type = BSP_UINT8;
  } else if (max_dim < (size_t)UINT16_MAX) {
    index_type = BSP_UINT16;
  } else if (max_dim < (size_t)UINT32_MAX) {
    index_type = BSP_UINT32;
  } else {
    index_type = BSP_UINT64;
  }

  return bsp_mmread_explicit(file_path, value_type, index_type);
}
