/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * binsparse_minimize_types.c - Minimize value/index types in a MATLAB
 * Binsparse matrix struct.
 *
 * Usage in MATLAB/Octave:
 *   matrix = binsparse_minimize_types(matrix)
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <complex.h>
#include <string.h>

#include "matlab_bsp_helpers.h"

static bool array_is_signed_integer(bsp_type_t type) {
  return type == BSP_INT8 || type == BSP_INT16 || type == BSP_INT32 ||
         type == BSP_INT64;
}

static bool array_is_integer(bsp_type_t type) {
  return type == BSP_UINT8 || type == BSP_UINT16 || type == BSP_UINT32 ||
         type == BSP_UINT64 || array_is_signed_integer(type);
}

static void minimize_iso_values(bsp_matrix_t* matrix) {
  if (matrix->is_iso || matrix->values.size == 0) {
    return;
  }

  bsp_array_t* values = &matrix->values;
  bool all_equal = true;

  switch (values->type) {
  case BSP_UINT8:
  case BSP_UINT16:
  case BSP_UINT32:
  case BSP_UINT64: {
    uint64_t first = 0;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      uint64_t current = 0;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_type_t out_type = values->type;
    if (first == 1) {
      out_type = BSP_BINT8;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, out_type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    if (out_type == BSP_BINT8) {
      int8_t one = 1;
      bsp_array_write(new_values, 0, one);
    } else {
      bsp_array_write(new_values, 0, first);
    }

    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  case BSP_INT8:
  case BSP_INT16:
  case BSP_INT32:
  case BSP_INT64:
  case BSP_BINT8: {
    int64_t first = 0;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      int64_t current = 0;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_type_t out_type = values->type;
    if (first == 1) {
      out_type = BSP_BINT8;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, out_type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    if (out_type == BSP_BINT8) {
      int8_t one = 1;
      bsp_array_write(new_values, 0, one);
    } else {
      bsp_array_write(new_values, 0, first);
    }

    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  case BSP_FLOAT32: {
    float first = 0.0f;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      float current = 0.0f;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, values->type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    bsp_array_write(new_values, 0, first);
    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  case BSP_FLOAT64: {
    double first = 0.0;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      double current = 0.0;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, values->type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    bsp_array_write(new_values, 0, first);
    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  case BSP_COMPLEX_FLOAT32: {
    float _Complex first = 0.0f + 0.0f * I;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      float _Complex current = 0.0f + 0.0f * I;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, values->type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    bsp_array_write(new_values, 0, first);
    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  case BSP_COMPLEX_FLOAT64: {
    double _Complex first = 0.0 + 0.0 * I;
    bsp_array_read((*values), 0, first);
    for (size_t i = 1; i < values->size; i++) {
      double _Complex current = 0.0 + 0.0 * I;
      bsp_array_read((*values), i, current);
      if (current != first) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal) {
      return;
    }

    bsp_array_t new_values;
    bsp_error_t error = bsp_construct_array_t_allocator(
        &new_values, 1, values->type, bsp_matlab_allocator);
    if (error != BSP_SUCCESS) {
      return;
    }

    bsp_array_write(new_values, 0, first);
    bsp_destroy_array_t(values);
    *values = new_values;
    matrix->is_iso = true;
    return;
  }
  default:
    return;
  }
}

static void minimize_int64_values(bsp_matrix_t* matrix) {
  int64_t* values = (int64_t*) matrix->values.data;

  int64_t min_value = values[0];
  int64_t max_value = values[0];

  for (size_t i = 1; i < matrix->values.size; i++) {
    if (values[i] > max_value) {
      max_value = values[i];
    }
    if (values[i] < min_value) {
      min_value = values[i];
    }
  }

  bsp_type_t value_type = BSP_INT64;
  if (min_value >= 0) {
    value_type = bsp_pick_integer_type((size_t) max_value);
  } else {
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

  if (value_type == matrix->values.type) {
    return;
  }

  bsp_array_t new_values;
  bsp_error_t error = bsp_construct_array_t_allocator(
      &new_values, matrix->values.size, value_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    return;
  }

  for (size_t i = 0; i < matrix->values.size; i++) {
    int64_t value = values[i];
    bsp_array_write(new_values, i, value);
  }

  bsp_destroy_array_t(&matrix->values);
  matrix->values = new_values;
}

static void minimize_values_matlab(bsp_matrix_t* matrix) {
  if (matrix->values.size == 0) {
    return;
  }

  if (matrix->values.type == BSP_FLOAT64) {
    double* values = (double*) matrix->values.data;
    bool float32_representable = true;
    for (size_t i = 0; i < matrix->values.size; i++) {
      if (((float) values[i]) != values[i]) {
        float32_representable = false;
        break;
      }
    }

    if (float32_representable) {
      bsp_array_t new_values;
      bsp_error_t error = bsp_construct_array_t_allocator(
          &new_values, matrix->values.size, BSP_FLOAT32, bsp_matlab_allocator);
      if (error != BSP_SUCCESS) {
        return;
      }

      float* n_values = (float*) new_values.data;
      for (size_t i = 0; i < matrix->values.size; i++) {
        n_values[i] = (float) values[i];
      }

      bsp_destroy_array_t(&matrix->values);
      matrix->values = new_values;
    }
  } else if (matrix->values.type == BSP_INT64) {
    minimize_int64_values(matrix);
  } else if (matrix->values.type == BSP_COMPLEX_FLOAT64) {
    bool float32_representable = true;
    double _Complex* values = (double _Complex*) matrix->values.data;

    for (size_t i = 0; i < matrix->values.size; i++) {
      if (((float _Complex) values[i]) != values[i]) {
        float32_representable = false;
        break;
      }
    }

    if (float32_representable) {
      bsp_array_t new_values;
      bsp_error_t error = bsp_construct_array_t_allocator(
          &new_values, matrix->values.size, BSP_COMPLEX_FLOAT32,
          bsp_matlab_allocator);
      if (error != BSP_SUCCESS) {
        return;
      }

      float _Complex* n_values = (float _Complex*) new_values.data;
      for (size_t i = 0; i < matrix->values.size; i++) {
        n_values[i] = (float _Complex) values[i];
      }

      bsp_destroy_array_t(&matrix->values);
      matrix->values = new_values;
    }
  }
}

static void minimize_index_array(bsp_array_t* array, const char* name) {
  if (array->size == 0) {
    return;
  }

  if (!array_is_integer(array->type)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidIndexType",
                      "%s must be an integer array", name);
  }

  size_t max_value = 0;

  if (array_is_signed_integer(array->type)) {
    for (size_t i = 0; i < array->size; i++) {
      int64_t value = 0;
      bsp_array_read((*array), i, value);
      if (value < 0) {
        mexErrMsgIdAndTxt("BinSparse:InvalidIndexValue",
                          "%s contains negative values", name);
      }
      if ((uint64_t) value > max_value) {
        max_value = (size_t) value;
      }
    }
  } else {
    for (size_t i = 0; i < array->size; i++) {
      uint64_t value = 0;
      bsp_array_read((*array), i, value);
      if (value > max_value) {
        max_value = (size_t) value;
      }
    }
  }

  bsp_type_t target_type = bsp_pick_integer_type(max_value);
  if (array->type == target_type) {
    return;
  }

  bsp_array_t new_array;
  bsp_error_t error = bsp_construct_array_t_allocator(
      &new_array, array->size, target_type, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError", "Failed to allocate %s array",
                      name);
  }

  for (size_t i = 0; i < array->size; i++) {
    uint64_t value = 0;
    bsp_array_read((*array), i, value);
    bsp_array_write(new_array, i, value);
  }

  bsp_destroy_array_t(array);
  *array = new_array;
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  if (nrhs != 1) {
    mexErrMsgIdAndTxt("BinSparse:InvalidArgs",
                      "Usage: matrix = binsparse_minimize_types(matrix)");
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs", "Too many output arguments");
  }

  if (!mxIsStruct(prhs[0])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidMatrix",
                      "Input must be a Binsparse matrix struct");
  }

  bsp_matrix_t matrix;
  bsp_error_t error = matlab_struct_to_bsp_matrix_allocator(
      prhs[0], &matrix, bsp_matlab_allocator);
  if (error != BSP_SUCCESS) {
    mexErrMsgIdAndTxt("BinSparse:ConversionError",
                      "Failed to convert MATLAB struct to matrix: %s",
                      bsp_get_error_string(error));
  }

  minimize_values_matlab(&matrix);
  minimize_index_array(&matrix.indices_0, "indices_0");
  minimize_index_array(&matrix.indices_1, "indices_1");
  minimize_index_array(&matrix.pointers_to_1, "pointers_to_1");
  minimize_iso_values(&matrix);

  plhs[0] = bsp_matrix_to_matlab_struct(&matrix);
  bsp_destroy_matrix_t(&matrix);
}
