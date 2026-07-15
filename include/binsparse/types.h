/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum bsp_type_t {
  BSP_UINT8 = 0,
  BSP_UINT16 = 1,
  BSP_UINT32 = 2,
  BSP_UINT64 = 3,
  BSP_INT8 = 4,
  BSP_INT16 = 5,
  BSP_INT32 = 6,
  BSP_INT64 = 7,
  BSP_FLOAT32 = 8,
  BSP_FLOAT64 = 9,
  BSP_BINT8 = 10,
  BSP_COMPLEX_FLOAT32 = 11,
  BSP_COMPLEX_FLOAT64 = 12,
  BSP_INVALID_TYPE = 13
} bsp_type_t;

static inline char* bsp_get_type_string(bsp_type_t type) {
  if (type == BSP_UINT8) {
    return (char*) "uint8";
  } else if (type == BSP_UINT16) {
    return (char*) "uint16";
  } else if (type == BSP_UINT32) {
    return (char*) "uint32";
  } else if (type == BSP_UINT64) {
    return (char*) "uint64";
  } else if (type == BSP_INT8) {
    return (char*) "int8";
  } else if (type == BSP_INT16) {
    return (char*) "int16";
  } else if (type == BSP_INT32) {
    return (char*) "int32";
  } else if (type == BSP_INT64) {
    return (char*) "int64";
  } else if (type == BSP_FLOAT32) {
    return (char*) "float32";
  } else if (type == BSP_FLOAT64) {
    return (char*) "float64";
  } else if (type == BSP_BINT8) {
    return (char*) "bint8";
  } else if (type == BSP_COMPLEX_FLOAT32) {
    return (char*) "complex[float32]";
  } else if (type == BSP_COMPLEX_FLOAT64) {
    return (char*) "complex[float64]";
  } else {
    return (char*) "";
  }
}

static inline size_t bsp_type_size(bsp_type_t type) {
  if (type == BSP_UINT8) {
    return sizeof(uint8_t);
  } else if (type == BSP_UINT16) {
    return sizeof(uint16_t);
  } else if (type == BSP_UINT32) {
    return sizeof(uint32_t);
  } else if (type == BSP_UINT64) {
    return sizeof(uint64_t);
  } else if (type == BSP_INT8) {
    return sizeof(int8_t);
  } else if (type == BSP_INT16) {
    return sizeof(int16_t);
  } else if (type == BSP_INT32) {
    return sizeof(int32_t);
  } else if (type == BSP_INT64) {
    return sizeof(int64_t);
  } else if (type == BSP_FLOAT32) {
    return sizeof(float);
  } else if (type == BSP_FLOAT64) {
    return sizeof(double);
  } else if (type == BSP_BINT8) {
    return sizeof(int8_t);
  } else if (type == BSP_COMPLEX_FLOAT32) {
    return sizeof(float _Complex);
  } else if (type == BSP_COMPLEX_FLOAT64) {
    return sizeof(double _Complex);
  } else {
    assert(false);
  }
}

// Given the maximum value `max_value` that must be stored,
// pick an unsigned integer type for indices.
static inline bsp_type_t bsp_pick_integer_type(size_t max_value) {
  if (max_value <= (size_t) UINT8_MAX) {
    return BSP_UINT8;
  } else if (max_value <= (size_t) UINT16_MAX) {
    return BSP_UINT16;
  } else if (max_value <= (size_t) UINT32_MAX) {
    return BSP_UINT32;
  } else {
    return BSP_UINT64;
  }
}
