/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <string.h>

typedef enum bsp_structure_t {
  BSP_GENERAL = 99,
  BSP_SYMMETRIC = 100,
  BSP_SYMMETRIC_LOWER = 100,
  BSP_SYMMETRIC_UPPER = 101,
  BSP_HERMITIAN = 102,
  BSP_HERMITIAN_LOWER = 102,
  BSP_HERMITIAN_UPPER = 103,
  BSP_SKEW_SYMMETRIC = 104,
  BSP_SKEW_SYMMETRIC_LOWER = 104,
  BSP_SKEW_SYMMETRIC_UPPER = 105,
  BSP_INVALID_STRUCTURE = 106
} bsp_structure_t;

char* bsp_get_structure_string(bsp_structure_t structure) {
  if (structure == BSP_GENERAL) {
    return (char*) "general";
  } else if (structure == BSP_SYMMETRIC_LOWER) {
    return (char*) "symmetric_lower";
  } else if (structure == BSP_SYMMETRIC_UPPER) {
    return (char*) "symmetric_upper";
  } else if (structure == BSP_HERMITIAN_LOWER) {
    return (char*) "hermitian_lower";
  } else if (structure == BSP_HERMITIAN_UPPER) {
    return (char*) "hermitian_upper";
  } else if (structure == BSP_SKEW_SYMMETRIC_LOWER) {
    return (char*) "skew_symmetric_lower";
  } else if (structure == BSP_SKEW_SYMMETRIC_UPPER) {
    return (char*) "skew_symmetric_upper";
  } else {
    return (char*) "";
  }
}

bsp_structure_t bsp_get_structure(char* structure) {
  if (strcmp(structure, "symmetric_lower") == 0) {
    return BSP_SYMMETRIC_LOWER;
  } else if (strcmp(structure, "symmetric_upper") == 0) {
    return BSP_SYMMETRIC_UPPER;
  } else if (strcmp(structure, "hermitian_lower") == 0) {
    return BSP_HERMITIAN_LOWER;
  } else if (strcmp(structure, "hermitian_upper") == 0) {
    return BSP_HERMITIAN_UPPER;
  } else if (strcmp(structure, "skew_symmetric_lower") == 0) {
    return BSP_SKEW_SYMMETRIC_LOWER;
  } else if (strcmp(structure, "skew_symmetric_upper") == 0) {
    return BSP_SKEW_SYMMETRIC_UPPER;
  } else {
    return BSP_INVALID_STRUCTURE;
  }
}
