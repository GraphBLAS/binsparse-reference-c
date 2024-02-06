#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  char* file_name = "test.hdf5";

  bsp_matrix_t mat = bsp_read_matrix(file_name);

  if (mat.format == BSP_COO) {
    float* values = mat.values.data;
    int* rowind = mat.indices_0.data;
    int* colind = mat.indices_1.data;

    for (size_t i = 0; i < mat.nnz; i++) {
      printf("%d, %d: %f\n", rowind[i], colind[i], values[i]);
    }
  }

  return 0;
}
