// SPDX-License-Identifier: BSD-3-Clause

#include <binsparse/binsparse.h>
#include <stdio.h>

int main(int argc, char** argv) {

  if (argc < 3) {
    printf(
        "usage: ./bsp2mtx [inputfile_name.mtx] [outputfile_name.bsp.hdf5]\n");
    return 1;
  }

  char* input_fname = argv[1];
  char* output_fname = argv[2];

  printf(" === Reading file... ===\n");
  bsp_matrix_t matrix = bsp_read_matrix(input_fname, NULL);
  printf(" === Done writing. ===\n");

  printf(" === Writing to %s... ===\n", output_fname);
  bsp_mmwrite(output_fname, matrix);
  printf(" === Done writing. ===\n");

  bsp_destroy_matrix_t(matrix);

  return 0;
}
