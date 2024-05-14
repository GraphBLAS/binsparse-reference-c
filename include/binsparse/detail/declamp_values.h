#pragma once

#include <binsparse/matrix.h>
#include <math.h>

// "Declamp" a matrix from the SuiteSparse Matrix Collection.
// SuiteSparse Matrix Collection clamps values at 1e308 and -1e308
// for printing.  These are almost all infinities or negative infinites.
// Here, we "declamp" these values to restore them to infinity.
// This allows the `bsp_matrix_minimize_values` to properly minimize
// matrices that have infinity values.
void bsp_matrix_declamp_values(bsp_matrix_t matrix) {
  const double HUGE_DOUBLE = 1e308;
  if (matrix.values.type == BSP_FLOAT64) {
    double* values = (double*) matrix.values.data;

    for (size_t i = 0; i < matrix.values.size; i++) {
      if (values[i] >= HUGE_DOUBLE) {
        values[i] = INFINITY;
      } else if (values[i] <= -HUGE_DOUBLE) {
        values[i] = -INFINITY;
      }
    }
  }
}
