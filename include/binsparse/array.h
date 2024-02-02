#pragma once

#include <binsparse/types.h>
#include <stdlib.h>

typedef struct bsp_array_t {
  void* data;
  size_t size;
  bsp_type_t type;
} bsp_array_t;

bsp_array_t bsp_construct_array_t(size_t size, bsp_type_t type) {
  size_t byte_size = size * bsp_type_size(type);

  bsp_array_t array;
  array.data = malloc(byte_size);
  array.size = size;
  array.type = type;

  return array;
}

void bsp_destroy_array_t(bsp_array_t array) {
  free(array.data);
}
