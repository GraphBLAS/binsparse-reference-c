// SPDX-FileCopyrightText: 2024 Binsparse Developers
//
// SPDX-License-Identifier: BSD-3-Clause

#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  const char* file_name = "test.hdf5";

  hid_t f = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);

  bsp_array_t array = bsp_read_array(f, "test");

  int* values = (int*) array.data;

  for (size_t i = 0; i < array.size; i++) {
    printf("%lu: %d\n", i, values[i]);
  }

  bsp_destroy_array_t(array);
  H5Fclose(f);

  return 0;
}
