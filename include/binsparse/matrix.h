/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/array.h>
#include <binsparse/matrix_formats.h>
#include <binsparse/structure.h>
#include <stdbool.h>

typedef struct bsp_matrix_t {
  bsp_array_t values;
  bsp_array_t indices_0;
  bsp_array_t indices_1;
  bsp_array_t pointers_to_1;

  size_t nrows;
  size_t ncols;
  size_t nnz;
  bool is_iso;

  bsp_matrix_format_t format;
  bsp_structure_t structure;
} bsp_matrix_t;

bsp_matrix_t bsp_construct_default_matrix_t() {
  bsp_matrix_t mat;
  mat.values = bsp_construct_default_array_t();
  mat.indices_0 = bsp_construct_default_array_t();
  mat.indices_1 = bsp_construct_default_array_t();
  mat.pointers_to_1 = bsp_construct_default_array_t();
  mat.nrows = mat.ncols = mat.nnz = 0;
  mat.is_iso = false;
  mat.structure = BSP_GENERAL;
  return mat;
}

void bsp_destroy_matrix_t(bsp_matrix_t matrix) {
  bsp_destroy_array_t(matrix.values);
  bsp_destroy_array_t(matrix.indices_0);
  bsp_destroy_array_t(matrix.indices_1);
  bsp_destroy_array_t(matrix.pointers_to_1);
}

size_t bsp_matrix_nbytes(bsp_matrix_t mat) {
  size_t nbytes = 0;
  if (mat.values.size > 0) {
    nbytes += mat.values.size * bsp_type_size(mat.values.type);
  }

  if (mat.indices_0.size > 0) {
    nbytes += mat.indices_0.size * bsp_type_size(mat.indices_0.type);
  }

  if (mat.indices_1.size > 0) {
    nbytes += mat.indices_1.size * bsp_type_size(mat.indices_1.type);
  }

  if (mat.pointers_to_1.size > 0) {
    nbytes += mat.pointers_to_1.size * bsp_type_size(mat.pointers_to_1.type);
  }

  return nbytes;
}

void bsp_print_matrix_info(bsp_matrix_t matrix) {
  printf("%lu x %lu matrix with %lu nnz.\n", matrix.nrows, matrix.ncols,
         matrix.nnz);
  printf("%s format with %s structure\n",
         bsp_get_matrix_format_string(matrix.format),
         bsp_get_structure_string(matrix.structure));
  if (matrix.values.data != NULL) {
    printf("%lu values of type %s\n", matrix.values.size,
           bsp_get_type_string(matrix.values.type));
  }

  if (matrix.indices_0.data != NULL) {
    printf("%lu indices_0 of type %s\n", matrix.indices_0.size,
           bsp_get_type_string(matrix.indices_0.type));
  }

  if (matrix.indices_1.data != NULL) {
    printf("%lu indices_1 of type %s\n", matrix.indices_1.size,
           bsp_get_type_string(matrix.indices_1.type));
  }

  if (matrix.pointers_to_1.data != NULL) {
    printf("%lu pointers_to_1 of type %s\n", matrix.pointers_to_1.size,
           bsp_get_type_string(matrix.pointers_to_1.type));
  }
}
