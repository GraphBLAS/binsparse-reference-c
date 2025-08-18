/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

typedef enum bsp_error_t {
  BSP_SUCCESS = 0,

  // Memory-related failures (malloc, array allocation, etc.)
  BSP_ERROR_MEMORY = 1,

  // File I/O failures (file not found, read/write errors, HDF5 operations)
  BSP_ERROR_IO = 2,

  // JSON parsing and metadata errors
  BSP_ERROR_FORMAT = 3,

  // Invalid input parameters, dimensions, types, etc.
  BSP_ERROR_INVALID_INPUT = 4,

  // Unsupported operations or not-yet-implemented features
  BSP_ERROR_UNSUPPORTED = 5,

  // Type-related errors (invalid types, type mismatches)
  BSP_ERROR_TYPE = 6,

  // Generic internal errors (should-never-happen cases)
  BSP_ERROR_INTERNAL = 7

} bsp_error_t;

/**
 * Get a human-readable error message for a bsp_error_t code.
 *
 * @param error The error code
 * @return A constant string describing the error
 */
static inline const char* bsp_get_error_string(bsp_error_t error) {
  switch (error) {
  case BSP_SUCCESS:
    return "Success";
  case BSP_ERROR_MEMORY:
    return "Memory allocation or management error";
  case BSP_ERROR_IO:
    return "File I/O or HDF5 operation error";
  case BSP_ERROR_FORMAT:
    return "JSON parsing or metadata format error";
  case BSP_ERROR_INVALID_INPUT:
    return "Invalid input parameters or data";
  case BSP_ERROR_UNSUPPORTED:
    return "Unsupported operation or feature";
  case BSP_ERROR_TYPE:
    return "Data type error or type mismatch";
  case BSP_ERROR_INTERNAL:
    return "Internal library error";
  default:
    return "Unknown error";
  }
}
