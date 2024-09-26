#pragma once

#include <stdlib.h>

typedef struct bsp_allocator_t {
  void* (*malloc)(size_t);
  void (*free)(void*);
} bsp_allocator_t;

const static bsp_allocator_t bsp_default_allocator = {.malloc = malloc,
                                                      .free = free};
