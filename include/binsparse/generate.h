#pragma once

#include <binsparse/matrix.h>

void bsp_array_fill_random(bsp_array_t array, size_t bound) {
  if (array.type == BSP_UINT8) {
    uint8_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT16) {
    uint16_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT32) {
    uint32_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_UINT64) {
    uint64_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT8) {
    int8_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT16) {
    int16_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT32) {
    int32_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_INT64) {
    int64_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % bound;
    }
  } else if (array.type == BSP_FLOAT32) {
    float* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = drand48() * bound;
    }
  } else if (array.type == BSP_FLOAT64) {
    double* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = drand48() * bound;
    }
  } else if (array.type == BSP_BINT8) {
    int8_t* values = array.data;
    for (size_t i = 0; i < array.size; i++) {
      values[i] = lrand48() % 2;
    }
  }
}

bsp_matrix_t bsp_generate_coo(size_t m, size_t n, size_t nnz, bsp_type_t value_type,
                          bsp_type_t index_type) {
  bsp_matrix_t matrix;
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