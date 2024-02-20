#pragma once

#include <binsparse/array.h>

typedef struct bsp_coo_indices_impl_ {
  bsp_array_t rowind;
  bsp_array_t colind;
} bsp_coo_indices_impl_;

static bsp_coo_indices_impl_ bsp_coo_indices_;

static int bsp_compare_int_impl_(size_t x, size_t y) {
  if (x < y) {
    return -1;
  } else if (x == y) {
    return 0;
  } else {
    return 1;
  }
}

static int bsp_coo_comparison_row_sort_operator_impl_(const void* x,
                                                      const void* y) {
  size_t x_index = *((const size_t*)x);
  size_t y_index = *((const size_t*)y);

  size_t x_i, x_j;
  size_t y_i, y_j;

  bsp_array_read(bsp_coo_indices_.rowind, x_index, x_i);
  bsp_array_read(bsp_coo_indices_.colind, x_index, x_j);

  bsp_array_read(bsp_coo_indices_.rowind, y_index, y_i);
  bsp_array_read(bsp_coo_indices_.colind, y_index, y_j);

  if (x_i != y_i) {
    return bsp_compare_int_impl_(x_i, y_i);
  } else {
    return bsp_compare_int_impl_(x_j, y_j);
  }
}
