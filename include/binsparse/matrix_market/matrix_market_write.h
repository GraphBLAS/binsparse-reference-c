#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <binsparse/matrix_market/matrix_market_type_t.h>

void bsp_mmwrite(char* file_path, bsp_matrix_t matrix) {
  FILE* f = fopen(file_path, "w");

  assert(f != NULL);

  char* structure = NULL;
  if (matrix.structure == BSP_GENERAL) {
    structure = "general";
  } else if (matrix.structure == BSP_SYMMETRIC) {
    structure = "symmetric";
  } else if (matrix.structure == BSP_HERMITIAN) {
    structure = "hermitian";
  } else if (matrix.structure == BSP_SKEW_SYMMETRIC) {
    structure = "skew-symmetric";
  } else {
    assert(false);
  }

  char* type = NULL;
  bsp_matrix_market_type_t mm_type = BSP_MM_REAL;

  // TODO: we perhaps don't want to use pattern for *all*
  //       iso matrices, but rather just those where the
  //       iso value is 1.

  if (matrix.is_iso == true) {
    mm_type = BSP_MM_PATTERN;
    type = "pattern";
  } else if ((matrix.values.type >= BSP_UINT8 &&
              matrix.values.type <= BSP_INT64) ||
             matrix.values.type == BSP_BINT8) {
    mm_type = BSP_MM_INTEGER;
    type = "integer";
  } else if (matrix.values.type >= BSP_FLOAT32 &&
             matrix.values.type <= BSP_FLOAT64) {
    mm_type = BSP_MM_REAL;
    type = "real";
  } else if (matrix.values.type == BSP_COMPLEX_FLOAT32 ||
             matrix.values.type == BSP_COMPLEX_FLOAT64) {
    mm_type = BSP_MM_COMPLEX;
    type = "complex";
  } else {
    assert(false);
  }

  fprintf(f, "%%%%MatrixMarket matrix coordinate %s %s\n", type, structure);

  fprintf(f, "%zu %zu %zu\n", matrix.nrows, matrix.ncols, matrix.nnz);

  assert(matrix.format == BSP_COO);
  if (matrix.format == BSP_COO) {
    for (size_t count = 0; count < matrix.nnz; count++) {
      if (mm_type == BSP_MM_PATTERN) {
        size_t i, j;
        bsp_array_read(matrix.indices_0, count, i);
        bsp_array_read(matrix.indices_1, count, j);
        fprintf(f, "%zu %zu\n", i + 1, j + 1);
      } else if (mm_type == BSP_MM_INTEGER) {
        size_t i, j;
        int64_t value;
        bsp_array_read(matrix.indices_0, count, i);
        bsp_array_read(matrix.indices_1, count, j);
        bsp_array_read(matrix.values, count, value);
        fprintf(f, "%zu %zu %lld\n", i + 1, j + 1, (long long) value);
      } else if (mm_type == BSP_MM_REAL) {
        size_t i, j;
        double value;
        bsp_array_read(matrix.indices_0, count, i);
        bsp_array_read(matrix.indices_1, count, j);
        bsp_array_read(matrix.values, count, value);
        fprintf(f, "%zu %zu %lf\n", i + 1, j + 1, value);
      } else if (mm_type == BSP_MM_COMPLEX) {
        size_t i, j;
        double _Complex value;
        bsp_array_read(matrix.indices_0, count, i);
        bsp_array_read(matrix.indices_1, count, j);
        bsp_array_read(matrix.values, count, value);
        double real_value = 1.0 * value;
        double complex_value = 1j * value;
        fprintf(f, "%zu %zu %lf %lf\n", i + 1, j + 1, real_value,
                complex_value);
      } else {
        assert(false);
      }
    }
  } else {
    assert(false);
  }

  fclose(f);
}
