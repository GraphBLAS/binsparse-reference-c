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

bsp_array_t bsp_complex_array_to_fp(bsp_array_t other) {
  assert(other.type == BSP_COMPLEX_FLOAT32 ||
         other.type == BSP_COMPLEX_FLOAT64);

  bsp_array_t array;
  array.data = other.data;
  array.size = other.size * 2;

  if (other.type == BSP_COMPLEX_FLOAT32) {
    array.type = BSP_FLOAT32;
  } else {
    array.type = BSP_FLOAT64;
  }

  return array;
}

bsp_array_t bsp_fp_array_to_complex(bsp_array_t other) {
  assert(other.type == BSP_FLOAT32 || other.type == BSP_FLOAT64);

  bsp_array_t array;
  array.data = other.data;
  array.size = other.size / 2;

  if (other.type == BSP_FLOAT32) {
    array.type = BSP_COMPLEX_FLOAT32;
  } else {
    array.type = BSP_COMPLEX_FLOAT64;
  }

  return array;
}

void bsp_destroy_array_t(bsp_array_t array) {
  free(array.data);
}

#ifndef __cplusplus

#define bsp_array_block(array, ptr, code_block)                                \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* ptr = (uint8_t*) array.data;                                    \
      code_block;                                                              \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* ptr = (uint16_t*) array.data;                                  \
      code_block;                                                              \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* ptr = (uint32_t*) array.data;                                  \
      code_block;                                                              \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* ptr = (uint64_t*) array.data;                                  \
      code_block;                                                              \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* ptr = (int8_t*) array.data;                                      \
      code_block;                                                              \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* ptr = (int16_t*) array.data;                                    \
      code_block;                                                              \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* ptr = (int32_t*) array.data;                                    \
      code_block;                                                              \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* ptr = (int64_t*) array.data;                                    \
      code_block;                                                              \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* ptr = (float*) array.data;                                        \
      code_block;                                                              \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* ptr = (float*) array.data;                                       \
      code_block;                                                              \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* ptr = (int8_t*) array.data;                                      \
      code_block;                                                              \
    } else if (array.type == BSP_COMPLEX_FLOAT32) {                            \
      float _Complex* ptr = (float _Complex*) array.data;                      \
      code_block;                                                              \
    } else if (array.type == BSP_COMPLEX_FLOAT64) {                            \
      double _Complex* ptr = (double _Complex*) array.data;                    \
      code_block;                                                              \
    }                                                                          \
  }

// array[index] = value
#define bsp_array_write(array, index, value)                                   \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = (uint8_t*) array.data;                                   \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = (uint16_t*) array.data;                                 \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = (uint32_t*) array.data;                                 \
      data[index] = value;                                                     \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = (uint64_t*) array.data;                                 \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = (int8_t*) array.data;                                     \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = (int16_t*) array.data;                                   \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = (int32_t*) array.data;                                   \
      data[index] = value;                                                     \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = (int64_t*) array.data;                                   \
      data[index] = value;                                                     \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = (float*) array.data;                                       \
      data[index] = value;                                                     \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = (double*) array.data;                                     \
      data[index] = value;                                                     \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = (int8_t*) array.data;                                     \
      data[index] = ((size_t) value) % 2;                                      \
    } else if (array.type == BSP_COMPLEX_FLOAT32) {                            \
      float _Complex* data = (float _Complex*) array.data;                     \
      data[index] = value;                                                     \
    } else if (array.type == BSP_COMPLEX_FLOAT64) {                            \
      double _Complex* data = (double _Complex*) array.data;                   \
      data[index] = value;                                                     \
    }                                                                          \
  }

// array_0[index_0] = array1[index_1]
#define bsp_array_awrite(array_0, index_0, array_1, index_1)                   \
  {                                                                            \
    if (array_0.type == BSP_UINT8) {                                           \
      uint8_t* data0 = (uint8_t*) array_0.data;                                \
      uint8_t* data1 = (uint8_t*) array_1.data;                                \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT16) {                                   \
      uint16_t* data0 = (uint16_t*) array_0.data;                              \
      uint16_t* data1 = (uint16_t*) array_1.data;                              \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT32) {                                   \
      uint32_t* data0 = (uint32_t*) array_0.data;                              \
      uint32_t* data1 = (uint32_t*) array_1.data;                              \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_UINT64) {                                   \
      uint64_t* data0 = (uint64_t*) array_0.data;                              \
      uint64_t* data1 = (uint64_t*) array_1.data;                              \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT8) {                                     \
      int8_t* data0 = (int8_t*) array_0.data;                                  \
      int8_t* data1 = (int8_t*) array_1.data;                                  \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT16) {                                    \
      int16_t* data0 = (int16_t*) array_0.data;                                \
      int16_t* data1 = (int16_t*) array_1.data;                                \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT32) {                                    \
      int32_t* data0 = (int32_t*) array_0.data;                                \
      int32_t* data1 = (int32_t*) array_1.data;                                \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_INT64) {                                    \
      int64_t* data0 = (int64_t*) array_0.data;                                \
      int64_t* data1 = (int64_t*) array_1.data;                                \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_FLOAT32) {                                  \
      float* data0 = (float*) array_0.data;                                    \
      float* data1 = (float*) array_1.data;                                    \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_FLOAT64) {                                  \
      double* data0 = (double*) array_0.data;                                  \
      double* data1 = (double*) array_1.data;                                  \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_BINT8) {                                    \
      int8_t* data0 = (int8_t*) array_0.data;                                  \
      int8_t* data1 = (int8_t*) array_1.data;                                  \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_COMPLEX_FLOAT32) {                          \
      float _Complex* data0 = (float _Complex*) array_0.data;                  \
      float _Complex* data1 = (float _Complex*) array_1.data;                  \
      data0[index_0] = data1[index_1];                                         \
    } else if (array_0.type == BSP_COMPLEX_FLOAT64) {                          \
      double _Complex* data0 = (double _Complex*) array_0.data;                \
      double _Complex* data1 = (double _Complex*) array_1.data;                \
      data0[index_0] = data1[index_1];                                         \
    }                                                                          \
  }

// value = array[index]
#define bsp_array_read(array, index, value)                                    \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = (uint8_t*) array.data;                                   \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = (uint16_t*) array.data;                                 \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = (uint32_t*) array.data;                                 \
      value = data[index];                                                     \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = (uint64_t*) array.data;                                 \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = (int8_t*) array.data;                                     \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = (int16_t*) array.data;                                   \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = (int32_t*) array.data;                                   \
      value = data[index];                                                     \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = (int64_t*) array.data;                                   \
      value = data[index];                                                     \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = (float*) array.data;                                       \
      value = data[index];                                                     \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = (double*) array.data;                                     \
      value = data[index];                                                     \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = (int8_t*) array.data;                                     \
      value = data[index];                                                     \
    } else if (array.type == BSP_COMPLEX_FLOAT32) {                            \
      float _Complex* data = (float _Complex*) array.data;                     \
      value = data[index];                                                     \
    } else if (array.type == BSP_COMPLEX_FLOAT64) {                            \
      double _Complex* data = (double _Complex*) array.data;                   \
      value = data[index];                                                     \
    }                                                                          \
  }

// swap(array[i], array[j])
#define bsp_array_swap(array, i, j)                                            \
  {                                                                            \
    if (array.type == BSP_UINT8) {                                             \
      uint8_t* data = (uint8_t*) array.data;                                   \
      uint8_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT16) {                                     \
      uint16_t* data = (uint16_t*) array.data;                                 \
      uint16_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT32) {                                     \
      uint32_t* data = (uint32_t*) array.data;                                 \
      uint32_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_UINT64) {                                     \
      uint64_t* data = (uint64_t*) array.data;                                 \
      uint64_t v = data[i];                                                    \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT8) {                                       \
      int8_t* data = (int8_t*) array.data;                                     \
      int8_t v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT16) {                                      \
      int16_t* data = (int16_t*) array.data;                                   \
      int16_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT32) {                                      \
      int32_t* data = (int32_t*) array.data;                                   \
      int32_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_INT64) {                                      \
      int64_t* data = (int64_t*) array.data;                                   \
      int64_t v = data[i];                                                     \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_FLOAT32) {                                    \
      float* data = (float*) array.data;                                       \
      float v = data[i];                                                       \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_FLOAT64) {                                    \
      double* data = (double*) array.data;                                     \
      double v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_BINT8) {                                      \
      int8_t* data = (int8_t*) array.data;                                     \
      int8_t v = data[i];                                                      \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_COMPLEX_FLOAT32) {                            \
      float _Complex* data = (float _Complex*) array.data;                     \
      float _Complex v = data[i];                                              \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    } else if (array.type == BSP_COMPLEX_FLOAT64) {                            \
      double _Complex* data = (double _Complex*) array.data;                   \
      double _Complex v = data[i];                                             \
      data[i] = data[j];                                                       \
      data[j] = v;                                                             \
    }                                                                          \
  }

#endif
