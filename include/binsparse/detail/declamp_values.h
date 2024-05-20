#pragma once

#include <binsparse/matrix.h>
#include <math.h>

double bsp_suitesparse_declamp_impl_(double value) {
  const double HUGE_DOUBLE = 1e308;
  if (value >= HUGE_DOUBLE) {
    return INFINITY;
  } else if (value <= -HUGE_DOUBLE) {
    return -INFINITY;
  } else {
    return value;
  }
}

// "Declamp" a matrix from the SuiteSparse Matrix Collection.
// SuiteSparse Matrix Collection clamps values at 1e308 and -1e308
// for printing.  These are almost all infinities or negative infinites.
// Here, we "declamp" these values to restore them to infinity.
// This allows the `bsp_matrix_minimize_values` to properly minimize
// matrices that have infinity values.
void bsp_matrix_declamp_values(bsp_matrix_t matrix) {
  if (matrix.values.type == BSP_FLOAT64) {
    double* values = (double*) matrix.values.data;

    for (size_t i = 0; i < matrix.values.size; i++) {
      values[i] = bsp_suitesparse_declamp_impl_(values[i]);
    }
  } else if (matrix.values.type == BSP_COMPLEX_FLOAT64) {
    double _Complex* values = (double _Complex*) matrix.values.data;

    for (size_t i = 0; i < matrix.values.size; i++) {
      double real_value = __real__ values[i];
      double imaginary_value = __imag__ values[i];
      real_value = bsp_suitesparse_declamp_impl_(real_value);
      imaginary_value = bsp_suitesparse_declamp_impl_(imaginary_value);

      values[i] = real_value + 1j * imaginary_value;
    }
  }
}
