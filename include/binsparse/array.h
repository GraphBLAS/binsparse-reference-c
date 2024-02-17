#pragma once

#include <assert.h>
#include <binsparse/types.h>
#include <stdlib.h>
#include <string.h>

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

bsp_array_t bsp_copy_construct_array_t(bsp_array_t other) {
  bsp_array_t array = bsp_construct_array_t(other.size, other.type);
  memcpy(array.data, other.data, other.size * bsp_type_size(other.type));

  return array;
}

void bsp_destroy_array_t(bsp_array_t array) {
  free(array.data);
}

// array[index] = value
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

// array_0[index_0] = array1[index_1]
#define bsp_array_awrite(array_0, index_0, array_1, index_1)                   \
  {                                                                            \
    if (array_0.type == BSP_UINT8) {                                           \
      uint8_t* data0 = array_0.data;                                           \
      uint8_t* data1 = array_1.data;                                           \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT16) {                                   \
      uint16_t* data0 = array_0.data;                                          \
      uint16_t* data1 = array_1.data;                                          \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT32) {                                   \
      uint32_t* data0 = array_0.data;                                          \
      uint32_t* data1 = array_1.data;                                          \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT64) {                                   \
      uint64_t* data0 = array_0.data;                                          \
      uint64_t* data1 = array_1.data;                                          \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT8) {                                     \
      int8_t* data0 = array_0.data;                                            \
      int8_t* data1 = array_1.data;                                            \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT16) {                                    \
      int16_t* data0 = array_0.data;                                           \
      int16_t* data1 = array_1.data;                                           \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT32) {                                    \
      int32_t* data0 = array_0.data;                                           \
      int32_t* data1 = array_1.data;                                           \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT64) {                                    \
      int64_t* data0 = array_0.data;                                           \
      int64_t* data1 = array_1.data;                                           \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_FLOAT32) {                                  \
      float* data0 = array_0.data;                                             \
      float* data1 = array_1.data;                                             \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_FLOAT64) {                                  \
      double* data0 = array_0.data;                                            \
      double* data1 = array_1.data;                                            \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_BINT8) {                                    \
      int8_t* data0 = array_0.data;                                            \
      int8_t* data1 = array_1.data;                                            \
      data0[index_0] = data1[index_1];                                         \
    }                                                                          \
  }

// value = array[index]
#define bsp_array_read(array, index, value)                                    \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = array.data;                                              \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = array.data;                                             \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = array.data;                                             \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = array.data;                                             \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = array.data;                                               \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = array.data;                                              \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = array.data;                                              \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = array.data;                                              \
      value = data[index];                                                     \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = array.data;                                                \
      value = data[index];                                                     \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = array.data;                                               \
      value = data[index];                                                     \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = array.data;                                               \
      value = data[index];                                                     \
    }                                                                          \
  }

// swap(array[i], array[j])
#define bsp_array_swap(array, i, j)                                            \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = array.data;                                              \
      uint8_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = array.data;                                             \
      uint16_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = array.data;                                             \
      uint32_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = array.data;                                             \
      uint64_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = array.data;                                               \
      int8_t v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = array.data;                                              \
      int16_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = array.data;                                              \
      int32_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = array.data;                                              \
      int64_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = array.data;                                                \
      float v = data[i];                                                       \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = array.data;                                               \
      double v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = array.data;                                               \
      int8_t v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    }                                                                          \
  }
