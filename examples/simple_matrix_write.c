#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  size_t m = 1000;
  size_t n = 1000;
  size_t nnz = 100;

  bsp_matrix_t mat = bsp_generate_coo(m, n, nnz, BSP_FLOAT32, BSP_INT32);

  float* values = mat.values.data;
  int* rowind = mat.indices_0.data;
  int* colind = mat.indices_1.data;

  for (size_t i = 0; i < nnz; i++) {
    printf("%d, %d: %f\n", rowind[i], colind[i], values[i]);
  }

  bsp_write_matrix("test.hdf5", mat);

  char* str = bsp_generate_json(mat);

  printf("%s\n", str);

  return 0;
}
