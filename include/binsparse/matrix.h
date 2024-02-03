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
