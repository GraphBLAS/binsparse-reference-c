#pragma once

#include <assert.h>
#include <binsparse/types.h>
#include <stdlib.h>

typedef struct bsp_array_t {
  void* data;
  size_t size;
  bsp_type_t type;
} bsp_array_t;

bsp_array_t bsp_construct_default_array_t() {
  bsp_array_t array;
  array.data = NULL;
  array.size = 0;
  return array;
}

bsp_array_t bsp_construct_array_t(size_t size, bsp_type_t type) {
  size_t byte_size = size * bsp_type_size(type);

  bsp_array_t array;
  array.data = malloc(byte_size);
  array.size = size;
  array.type = type;

  return array;
}

void bsp_destroy_array_t(bsp_array_t array) {
  free(array.data);
}

#define bsp_array_write(array, index, value)                                   \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = array.data;                                              \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = array.data;                                             \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = array.data;                                             \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = array.data;                                             \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = array.data;                                               \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = array.data;                                              \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = array.data;                                              \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = array.data;                                              \
      data[index] = value;                                                     \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = array.data;                                                \
      data[index] = value;                                                     \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = array.data;                                               \
      data[index] = value;                                                     \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = array.data;                                               \
      data[index] = ((size_t)value) % 2;                                       \
    }                                                                          \
  }
