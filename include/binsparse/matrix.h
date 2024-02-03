#pragma once

#include <binsparse/array.h>
#include <binsparse/matrix_formats.h>

typedef struct bsp_matrix_t {
  bsp_array_t values;
  bsp_array_t indices_0;
  bsp_array_t indices_1;
  bsp_array_t pointers_to_1;

  size_t nrows;
  size_t ncols;
  size_t nnz;

  bsp_matrix_format_t format;
} bsp_matrix_t;

bsp_matrix_t bsp_construct_default_matrix_t() {
  bsp_matrix_t mat;
  mat.values = bsp_construct_default_array_t();
  mat.indices_0 = bsp_construct_default_array_t();
  mat.indices_1 = bsp_construct_default_array_t();
  mat.pointers_to_1 = bsp_construct_default_array_t();
  mat.nrows = mat.ncols = mat.nnz = 0;
  return mat;
}

void bsp_destroy_matrix_t(bsp_matrix_t matrix) {
  bsp_destroy_array_t(matrix.values);
  bsp_destroy_array_t(matrix.indices_0);
  bsp_destroy_array_t(matrix.indices_1);
  bsp_destroy_array_t(matrix.pointers_to_1);
}
