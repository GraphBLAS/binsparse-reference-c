/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <binsparse/read_tensor.h>
#include <binsparse/tensor.h>
#include <binsparse/write_tensor.h>

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr,
            "usage: ./tensor_test [file_name.h5] [output_file_name.h5]\n");
    return 1;
  }
  char* file_name = argv[1];
  bsp_tensor_t tensor = bsp_read_tensor(argv[1], NULL);
  printf("rank: %d\n", tensor.rank);
  printf("dims:");
  for (int i = 0; i < tensor.rank; i++) {
    printf("%ld, ", tensor.dims[i]);
  }
  printf("\n");
  bsp_write_tensor(argv[2], tensor, NULL, NULL, 9);
  bsp_destroy_tensor_t(tensor);
  return 0;
}
