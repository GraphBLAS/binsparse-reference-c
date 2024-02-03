#pragma once

#include <binsparse/matrix.h>

int bsp_write_matrix(char* file_name, bsp_matrix_t matrix) {
  hid_t f = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  if (matrix.format == BSP_COOR || matrix.format == BSP_COOC) {
    int result = bsp_write_array(f, "values", matrix.values);

    if (result != 0)
      return result;

    result = bsp_write_array(f, "indices_0", matrix.indices_0);

    if (result != 0)
      return result;

    result = bsp_write_array(f, "indices_1", matrix.indices_1);

    if (result != 0)
      return result;
  } else {
    return -1;
  }

  H5Fclose(f);
  return 0;
}