/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <binsparse/matrix.h>
#include <binsparse/matrix_market/coo_sort_tools.h>
#include <stdlib.h>
#include <string.h>

static inline bsp_error_t
bsp_copy_construct_array_t_allocator(bsp_array_t* array, bsp_array_t other,
                                     bsp_allocator_t allocator) {
  bsp_error_t error =
      bsp_construct_array_t_allocator(array, other.size, other.type, allocator);
  if (error != BSP_SUCCESS) {
    return error;
  }

  memcpy(array->data, other.data, other.size * bsp_type_size(other.type));
  return BSP_SUCCESS;
}

static inline bsp_matrix_t
bsp_convert_matrix_allocator(bsp_matrix_t matrix, bsp_matrix_format_t format,
                             bsp_allocator_t allocator) {
  // Throw an error if matrix already in desired format.
  if (matrix.format == format) {
    assert(false);
  }

  if (format == BSP_COOR) {
    // *Convert to COO* from another format.
    if (matrix.format == BSP_CSR) {
      // Convert CSR -> COOR
      bsp_matrix_t result;
      bsp_construct_default_matrix_t_allocator(&result, allocator);

      result.format = BSP_COOR;

      // Inherit NNZ, nrows, ncols, ISO-ness, and structure directly from
      // original matrix.
      result.nnz = matrix.nnz;
      result.nrows = matrix.nrows;
      result.ncols = matrix.ncols;
      result.is_iso = matrix.is_iso;
      result.structure = matrix.structure;

      size_t max_dim =
          (matrix.nrows > matrix.ncols) ? matrix.nrows : matrix.ncols;

      bsp_type_t index_type = bsp_pick_integer_type(max_dim);

      bsp_error_t error = bsp_copy_construct_array_t_allocator(
          &result.values, matrix.values, allocator);
      if (error != BSP_SUCCESS) {
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      // There is a corner case with tall and skinny matrices where we need a
      // higher width for rowind.  In order to keep rowind/colind the same type,
      // we might upcast.

      if (index_type == matrix.indices_1.type) {
        error = bsp_copy_construct_array_t_allocator(
            &result.indices_1, matrix.indices_1, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }
      } else {
        error = bsp_construct_array_t_allocator(
            &result.indices_1, matrix.indices_1.size, index_type, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        for (size_t i = 0; i < matrix.indices_1.size; i++) {
          size_t index;
          bsp_array_read(matrix.indices_1, i, index);
          bsp_array_write(result.indices_1, i, index);
        }
      }

      error = bsp_construct_array_t_allocator(&result.indices_0, matrix.nnz,
                                              index_type, allocator);
      if (error != BSP_SUCCESS) {
        bsp_destroy_array_t(&result.values);
        bsp_destroy_array_t(&result.indices_1);
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      for (size_t i = 0; i < matrix.nrows; i++) {
        size_t row_begin, row_end;
        bsp_array_read(matrix.pointers_to_1, i, row_begin);
        bsp_array_read(matrix.pointers_to_1, i + 1, row_end);
        for (size_t j_ptr = row_begin; j_ptr < row_end; j_ptr++) {
          bsp_array_write(result.indices_0, j_ptr, i);
        }
      }
      return result;
    } else if (matrix.format == BSP_CSC) {
      // Convert CSC -> COOR
      bsp_matrix_t result;
      bsp_construct_default_matrix_t_allocator(&result, allocator);

      result.format = BSP_COOR;

      // Inherit NNZ, nrows, ncols, ISO-ness, and structure directly from
      // original matrix.
      result.nnz = matrix.nnz;
      result.nrows = matrix.nrows;
      result.ncols = matrix.ncols;
      result.is_iso = matrix.is_iso;
      result.structure = matrix.structure;

      size_t max_dim =
          (matrix.nrows > matrix.ncols) ? matrix.nrows : matrix.ncols;

      bsp_type_t index_type = bsp_pick_integer_type(max_dim);

      bsp_error_t error = bsp_copy_construct_array_t_allocator(
          &result.values, matrix.values, allocator);
      if (error != BSP_SUCCESS) {
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      // Copy row indices from CSC to become row indices in COOR.
      if (index_type == matrix.indices_1.type) {
        error = bsp_copy_construct_array_t_allocator(
            &result.indices_0, matrix.indices_1, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }
      } else {
        error = bsp_construct_array_t_allocator(
            &result.indices_0, matrix.indices_1.size, index_type, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        for (size_t i = 0; i < matrix.indices_1.size; i++) {
          size_t index;
          bsp_array_read(matrix.indices_1, i, index);
          bsp_array_write(result.indices_0, i, index);
        }
      }

      // Generate column indices by expanding column pointers.
      error = bsp_construct_array_t_allocator(&result.indices_1, matrix.nnz,
                                              index_type, allocator);
      if (error != BSP_SUCCESS) {
        bsp_destroy_array_t(&result.values);
        bsp_destroy_array_t(&result.indices_0);
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      for (size_t j = 0; j < matrix.ncols; j++) {
        size_t col_begin, col_end;
        bsp_array_read(matrix.pointers_to_1, j, col_begin);
        bsp_array_read(matrix.pointers_to_1, j + 1, col_end);
        for (size_t i_ptr = col_begin; i_ptr < col_end; i_ptr++) {
          bsp_array_write(result.indices_1, i_ptr, j);
        }
      }

      // Sort the result by rows to produce valid COOR.
      size_t* indices = (size_t*) malloc(sizeof(size_t) * matrix.nnz);
      if (indices == NULL) {
        bsp_destroy_array_t(&result.values);
        bsp_destroy_array_t(&result.indices_0);
        bsp_destroy_array_t(&result.indices_1);
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      for (size_t i = 0; i < matrix.nnz; i++) {
        indices[i] = i;
      }

      bsp_coo_indices_.rowind = result.indices_0;
      bsp_coo_indices_.colind = result.indices_1;

      qsort(indices, matrix.nnz, sizeof(size_t),
            bsp_coo_comparison_row_sort_operator_impl_);

      bsp_array_t rowind;
      bsp_array_t colind;

      error = bsp_copy_construct_array_t_allocator(&rowind, result.indices_0,
                                                   allocator);
      if (error != BSP_SUCCESS) {
        free(indices);
        bsp_destroy_array_t(&result.values);
        bsp_destroy_array_t(&result.indices_0);
        bsp_destroy_array_t(&result.indices_1);
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      error = bsp_copy_construct_array_t_allocator(&colind, result.indices_1,
                                                   allocator);
      if (error != BSP_SUCCESS) {
        bsp_destroy_array_t(&rowind);
        free(indices);
        bsp_destroy_array_t(&result.values);
        bsp_destroy_array_t(&result.indices_0);
        bsp_destroy_array_t(&result.indices_1);
        bsp_matrix_t empty_result;
        bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
        return empty_result;
      }

      bsp_array_t values;

      if (!result.is_iso) {
        error = bsp_copy_construct_array_t_allocator(&values, result.values,
                                                     allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&rowind);
          bsp_destroy_array_t(&colind);
          free(indices);
          bsp_destroy_array_t(&result.values);
          bsp_destroy_array_t(&result.indices_0);
          bsp_destroy_array_t(&result.indices_1);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }
      }

      for (size_t i = 0; i < matrix.nnz; i++) {
        bsp_array_awrite(rowind, i, result.indices_0, indices[i]);
        bsp_array_awrite(colind, i, result.indices_1, indices[i]);
        if (!result.is_iso) {
          bsp_array_awrite(values, i, result.values, indices[i]);
        }
      }

      bsp_destroy_array_t(&result.indices_0);
      bsp_destroy_array_t(&result.indices_1);
      result.indices_0 = rowind;
      result.indices_1 = colind;

      if (!result.is_iso) {
        bsp_destroy_array_t(&result.values);
        result.values = values;
      }

      free(indices);
      return result;
    } else {
      assert(false);
    }
  } else {
    // Convert to any another format.

    // Currently only support COOR -> X.
    // If matrix is not COOR, convert to COOR.
    if (matrix.format != BSP_COOR) {
      bsp_matrix_t intermediate =
          bsp_convert_matrix_allocator(matrix, BSP_COOR, allocator);
      bsp_matrix_t result =
          bsp_convert_matrix_allocator(intermediate, format, allocator);
      bsp_destroy_matrix_t(&intermediate);
      return result;
    } else {
      if (format == BSP_CSR) {
        // Convert COOR -> CSR

        bsp_matrix_t result;
        bsp_construct_default_matrix_t_allocator(&result, allocator);

        result.format = BSP_CSR;

        result.nrows = matrix.nrows;
        result.ncols = matrix.ncols;
        result.nnz = matrix.nnz;
        result.is_iso = matrix.is_iso;
        result.structure = matrix.structure;

        // TODO: consider whether to produce files with varying integer types
        //       for row indices, column indices, and offsets.

        size_t max_dim =
            (matrix.nrows > matrix.ncols) ? matrix.nrows : matrix.ncols;

        size_t max_value =
            (max_dim > matrix.values.size) ? max_dim : matrix.values.size;

        bsp_type_t value_type = matrix.values.type;
        bsp_type_t index_type = bsp_pick_integer_type(max_value);

        // Since COOR is sorted by rows and then by columns, values and column
        // indices can be copied exactly.  Values' type will not change, but
        // column indices might, thus the extra branch.

        bsp_error_t error = bsp_copy_construct_array_t_allocator(
            &result.values, matrix.values, allocator);
        if (error != BSP_SUCCESS) {
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        if (index_type == matrix.indices_1.type) {
          error = bsp_copy_construct_array_t_allocator(
              &result.indices_1, matrix.indices_1, allocator);
          if (error != BSP_SUCCESS) {
            bsp_destroy_array_t(&result.values);
            bsp_matrix_t empty_result;
            bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
            return empty_result;
          }
        } else {
          error = bsp_construct_array_t_allocator(&result.indices_1, matrix.nnz,
                                                  index_type, allocator);
          if (error != BSP_SUCCESS) {
            bsp_destroy_array_t(&result.values);
            bsp_matrix_t empty_result;
            bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
            return empty_result;
          }

          for (size_t i = 0; i < matrix.nnz; i++) {
            size_t index;
            bsp_array_read(matrix.indices_1, i, index);
            bsp_array_write(result.indices_1, i, index);
          }
        }

        error = bsp_construct_array_t_allocator(
            &result.pointers_to_1, matrix.nrows + 1, index_type, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_destroy_array_t(&result.indices_1);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        bsp_array_t rowptr = result.pointers_to_1;

        bsp_array_write(rowptr, 0, 0);

        size_t r = 0;
        size_t c = 0;
        for (size_t c = 0; c < matrix.nnz; c++) {
          size_t j;
          bsp_array_read(matrix.indices_0, c, j);

          while (r < j) {
            assert(r + 1 <= matrix.nrows);

            bsp_array_write(rowptr, r + 1, c);
            r++;
          }
        }

        for (; r < matrix.nrows; r++) {
          bsp_array_write(rowptr, r + 1, matrix.nnz);
        }

        return result;
      } else if (format == BSP_CSC) {
        // Convert COOR -> CSC

        // First, sort by columns to prepare for CSC format.
        size_t* indices = (size_t*) malloc(sizeof(size_t) * matrix.nnz);
        if (indices == NULL) {
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        for (size_t i = 0; i < matrix.nnz; i++) {
          indices[i] = i;
        }

        bsp_coo_indices_.rowind = matrix.indices_0;
        bsp_coo_indices_.colind = matrix.indices_1;

        qsort(indices, matrix.nnz, sizeof(size_t),
              bsp_coo_comparison_col_sort_operator_impl_);

        bsp_matrix_t result;
        bsp_construct_default_matrix_t_allocator(&result, allocator);

        result.format = BSP_CSC;

        result.nrows = matrix.nrows;
        result.ncols = matrix.ncols;
        result.nnz = matrix.nnz;
        result.is_iso = matrix.is_iso;
        result.structure = matrix.structure;

        size_t max_dim =
            (matrix.nrows > matrix.ncols) ? matrix.nrows : matrix.ncols;

        size_t max_value =
            (max_dim > matrix.values.size) ? max_dim : matrix.values.size;

        bsp_type_t index_type = bsp_pick_integer_type(max_value);

        // Reorder values according to column-major sort.
        bsp_error_t error = bsp_construct_array_t_allocator(
            &result.values, matrix.values.size, matrix.values.type, allocator);
        if (error != BSP_SUCCESS) {
          free(indices);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        for (size_t i = 0; i < matrix.values.size; i++) {
          bsp_array_awrite(result.values, i, matrix.values,
                           matrix.is_iso ? 0 : indices[i]);
        }

        // Reorder row indices according to column-major sort.
        error = bsp_construct_array_t_allocator(&result.indices_1, matrix.nnz,
                                                index_type, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          free(indices);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        for (size_t i = 0; i < matrix.nnz; i++) {
          bsp_array_awrite(result.indices_1, i, matrix.indices_0, indices[i]);
        }

        // Build column pointers.
        error = bsp_construct_array_t_allocator(
            &result.pointers_to_1, matrix.ncols + 1, index_type, allocator);
        if (error != BSP_SUCCESS) {
          bsp_destroy_array_t(&result.values);
          bsp_destroy_array_t(&result.indices_1);
          free(indices);
          bsp_matrix_t empty_result;
          bsp_construct_default_matrix_t_allocator(&empty_result, allocator);
          return empty_result;
        }

        bsp_array_t colptr = result.pointers_to_1;

        bsp_array_write(colptr, 0, 0);

        size_t c = 0;
        for (size_t i = 0; i < matrix.nnz; i++) {
          size_t j;
          bsp_array_read(matrix.indices_1, indices[i], j);

          while (c < j) {
            assert(c + 1 <= matrix.ncols);

            bsp_array_write(colptr, c + 1, i);
            c++;
          }
        }

        for (; c < matrix.ncols; c++) {
          bsp_array_write(colptr, c + 1, matrix.nnz);
        }

        free(indices);
        return result;
      } else {
        assert(false);
      }
    }
  }
}

static inline bsp_matrix_t bsp_convert_matrix(bsp_matrix_t matrix,
                                              bsp_matrix_format_t format) {
  return bsp_convert_matrix_allocator(matrix, format, bsp_default_allocator);
}
