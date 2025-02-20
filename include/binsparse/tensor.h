#pragma once

#include <binsparse/array.h>
#include <binsparse/structure.h>

typedef enum {
  BSP_TENSOR_SPARSE = 0,
  BSP_TENSOR_DENSE = 1,
  BSP_TENSOR_ELEMENT = 2,
} bsp_level_kind_t;

typedef struct {
  bsp_level_kind_t kind;
  // data here should be bsp_element_t*, bsp_sparse_t*, or bsp_dense_t*
  void* data;
} bsp_level_t;

// corresponds to BSP_TENSOR_ELEMENT
typedef struct {
  bsp_array_t values;
} bsp_element_t;

// corresponds to BSP_TENSOR_DENSE
typedef struct {
  int rank;
  bsp_array_t* pointers_to;
  bsp_array_t* indices;
  bsp_level_t* child;
} bsp_sparse_t;

typedef struct {
  int rank;
  bsp_level_t* child;
} bsp_dense_t;

typedef struct {
  int rank;
  size_t* dims;
  size_t nnz;
  bool is_iso;

  bsp_level_t* level;
  // don't think too much about this at the moment.
  bsp_structure_t structure;
} bsp_tensor_t;

static inline bsp_tensor_t bsp_construct_default_tensor_t() {
  bsp_tensor_t tensor;
  tensor.structure = BSP_GENERAL;
  tensor.is_iso = false;
  tensor.nnz = tensor.rank = 0;
  tensor.dims = NULL;

  tensor.level = NULL;
  return tensor;
}

static void bsp_destroy_level_t(bsp_level_t* level) {
  if (level == NULL)
    return;
  switch (level->kind) {
  case BSP_TENSOR_ELEMENT:;
    bsp_element_t* element = level->data;
    bsp_destroy_array_t(element->values);
    free(element);
    break;
  case BSP_TENSOR_DENSE:;
    bsp_dense_t* dense = level->data;
    bsp_destroy_level_t(dense->child);
    free(dense);
    break;
  case BSP_TENSOR_SPARSE:;
    bsp_sparse_t* sparse = level->data;

    if (sparse->pointers_to != NULL)
      bsp_destroy_array_t(*sparse->pointers_to);
    if (sparse->indices != NULL) {
      for (int i = 0; i < sparse->rank; i++) {
        bsp_destroy_array_t(sparse->indices[i]);
      }
    }
    bsp_destroy_level_t(sparse->child);
    free(sparse);
    break;
  default:;
  }
}

static bsp_array_t bsp_get_tensor_values(bsp_tensor_t tensor) {
  bsp_level_t* level = tensor.level;
  while (level != NULL) {
    switch (level->kind) {
    case BSP_TENSOR_ELEMENT:;
      bsp_element_t* element = level->data;
      return element->values;
      break;
    case BSP_TENSOR_SPARSE:;
      bsp_sparse_t* sparse = level->data;
      level = sparse->child;
      break;
    case BSP_TENSOR_DENSE:;
      bsp_dense_t* dense = level->data;
      level = dense->child;
      break;
    default:;
    }
  }
  // this should never happen!
  assert(false);
}

static inline void bsp_destroy_tensor_t(bsp_tensor_t tensor) {
  bsp_destroy_level_t(tensor.level);
  free(tensor.dims);
}
