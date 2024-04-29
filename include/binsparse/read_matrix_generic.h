#pragma once

#include <binsparse/matrix_market/matrix_market_read.h>
#include <binsparse/read_matrix.h>
#include <string.h>

size_t bsp_final_dot(char* str) {
  size_t dot_idx = 0;
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (str[i] == '.') {
      dot_idx = i;
    }
  }
  return dot_idx;
}

bsp_matrix_t bsp_read_matrix_generic(char* file_name) {
  size_t idx = bsp_final_dot(file_name);

  if (strcmp(file_name + idx, ".hdf5") == 0 ||
      strcmp(file_name + idx, ".h5") == 0) {
    return bsp_read_matrix(file_name);
  } else if (strcmp(file_name + idx, ".mtx") == 0) {
    return bsp_mmread(file_name);
  } else {
    assert(false);
  }
}
