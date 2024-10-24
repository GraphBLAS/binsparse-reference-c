/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/matrix.h>

inline void bsp_array_fill_random(bsp_array_t array, size_t bound) {
  if (array.type == BSP_UINT8) {
    uint8_t* values = (uint8_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT16) {
    uint16_t* values = (uint16_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT32) {
    uint32_t* values = (uint32_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT64) {
    uint64_t* values = (uint64_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT8) {
    int8_t* values = (int8_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT16) {
    int16_t* values = (int16_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT32) {
    int32_t* values = (int32_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT64) {
    int64_t* values = (int64_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_FLOAT32) {
    float* values = (float*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = drand48() * bound;
    }
  } else if (array.type == BSP_FLOAT64) {
    double* values = (double*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = drand48() * bound;
    }
  } else if (array.type == BSP_BINT8) {
    int8_t* values = (int8_t*) array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % 2;
    }
  }
}

inline bsp_matrix_t bsp_generate_coo(size_t m, size_t n, size_t nnz,
                                     bsp_type_t value_type,
                                     bsp_type_t index_type) {
  bsp_matrix_t matrix = bsp_construct_default_matrix_t();
  matrix.nrows = m;
  matrix.ncols = n;
  matrix.nnz = nnz;
  matrix.values = bsp_construct_array_t(nnz, value_type);
  matrix.indices_0 = bsp_construct_array_t(nnz, index_type);
  matrix.indices_1 = bsp_construct_array_t(nnz, index_type);
  matrix.format = BSP_COO;

  bsp_array_fill_random(matrix.values, 100);
  bsp_array_fill_random(matrix.indices_0, m);
  bsp_array_fill_random(matrix.indices_1, n);

  return matrix;
}
