/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/matrix.h>
#include <stdbool.h>

// Given a matrix `matrix`, store its values using the smallest type possible
// without losing precision.
static inline bsp_matrix_t bsp_matrix_minimize_values(bsp_matrix_t matrix) {
  if (matrix.values.type == BSP_FLOAT64) {
    bool float32_representable = true;

    double* values = (double*) matrix.values.data;

    for (size_t i = 0; i < matrix.values.size; i++) {
      if (((float) values[i]) != values[i]) {
        float32_representable = false;
      }
    }

    if (float32_representable) {
      bsp_array_t new_values =
          bsp_construct_array_t(matrix.values.size, BSP_FLOAT32);

      float* n_values = (float*) new_values.data;

      for (size_t i = 0; i < matrix.values.size; i++) {
        n_values[i] = values[i];
      }

      bsp_destroy_array_t(matrix.values);
      matrix.values = new_values;
    }
  } else if (matrix.values.type == BSP_INT64) {
    int64_t* values = (int64_t*) matrix.values.data;

    int64_t min_value = values[0];
    int64_t max_value = values[0];

    for (size_t i = 1; i < matrix.values.size; i++) {
      if (values[i] > max_value) {
        max_value = values[i];
      }

      if (values[i] < min_value) {
        min_value = values[i];
      }
    }

    bsp_type_t value_type;
    if (min_value >= 0) {
      // No negative values => unsigned integers
      if (max_value <= (int64_t) UINT8_MAX) {
        value_type = BSP_UINT8;
      } else if (max_value <= (int64_t) UINT16_MAX) {
        value_type = BSP_UINT16;
      } else if (max_value <= (int64_t) UINT32_MAX) {
        value_type = BSP_UINT32;
      } else {
        value_type = BSP_UINT64;
      }
    } else {
      // Negative values => signed integers
      if (max_value <= (int64_t) INT8_MAX && min_value >= (int64_t) INT8_MIN) {
        value_type = BSP_INT8;
      } else if (max_value <= (int64_t) INT16_MAX &&
                 min_value >= (int64_t) INT16_MIN) {
        value_type = BSP_INT16;
      } else if (max_value <= (int64_t) INT32_MAX &&
                 min_value >= (int64_t) INT32_MIN) {
        value_type = BSP_INT32;
      } else {
        value_type = BSP_INT64;
      }
    }
    bsp_array_t new_values =
        bsp_construct_array_t(matrix.values.size, value_type);

    for (size_t i = 0; i < matrix.values.size; i++) {
      int64_t value;
      bsp_array_read(matrix.values, i, value);
      bsp_array_write(new_values, i, value);
    }

    bsp_destroy_array_t(matrix.values);
    matrix.values = new_values;
  } else if (matrix.values.type == BSP_COMPLEX_FLOAT64) {
    bool float32_representable = true;

    double _Complex* values = (double _Complex*) matrix.values.data;

    for (size_t i = 0; i < matrix.values.size; i++) {
      if (((float _Complex) values[i]) != values[i]) {
        float32_representable = false;
      }
    }

    if (float32_representable) {
      bsp_array_t new_values =
          bsp_construct_array_t(matrix.values.size, BSP_COMPLEX_FLOAT32);

      float _Complex* n_values = (float _Complex*) new_values.data;

      for (size_t i = 0; i < matrix.values.size; i++) {
        n_values[i] = values[i];
      }

      bsp_destroy_array_t(matrix.values);
      matrix.values = new_values;
    }
  }

  return matrix;
}
