// SPDX-License-Identifier: BSD-3-Clause

#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  size_t m = 1000;
  size_t n = 1000;
  size_t nnz = 100;

  bsp_matrix_t mat = bsp_generate_coo(m, n, nnz, BSP_FLOAT32, BSP_INT32);

  float* values = (float*) mat.values.data;
  int* rowind = (int*) mat.indices_0.data;
  int* colind = (int*) mat.indices_1.data;

  for (size_t i = 0; i < nnz; i++) {
    printf("%d, %d: %f\n", rowind[i], colind[i], values[i]);
  }

  bsp_write_matrix("test.hdf5", mat, NULL, NULL, 9);

  return 0;
}
