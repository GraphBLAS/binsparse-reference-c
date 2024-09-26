/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <binsparse/matrix.h>

bsp_matrix_t bsp_convert_matrix(bsp_matrix_t matrix,
                                bsp_matrix_format_t format) {
  // Throw an error if matrix already in desired format.
  if (matrix.format == format) {
    assert(false);
  }

  if (format == BSP_COOR) {
    // *Convert to COO* from another format.
    if (matrix.format == BSP_CSR) {
      // Convert CSR -> COOR
      bsp_matrix_t result = bsp_construct_default_matrix_t();

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

      result.values = bsp_copy_construct_array_t(matrix.values);

      // There is a corner case with tall and skinny matrices where we need a
      // higher width for rowind.  In order to keep rowind/colind the same type,
      // we might upcast.

      if (index_type == matrix.indices_1.type) {
        result.indices_1 = bsp_copy_construct_array_t(matrix.indices_1);
      } else {
        result.indices_1 =
            bsp_construct_array_t(matrix.indices_1.size, index_type);
        for (size_t i = 0; i < matrix.indices_1.size; i++) {
          size_t index;
          bsp_array_read(matrix.indices_1, i, index);
          bsp_array_write(result.indices_1, i, index);
        }
      }

      result.indices_0 = bsp_construct_array_t(matrix.nnz, index_type);

      for (size_t i = 0; i < matrix.nrows; i++) {
        size_t row_begin, row_end;
        bsp_array_read(matrix.pointers_to_1, i, row_begin);
        bsp_array_read(matrix.pointers_to_1, i + 1, row_end);
        for (size_t j_ptr = row_begin; j_ptr < row_end; j_ptr++) {
          bsp_array_write(result.indices_0, j_ptr, i);
        }
      }
      return result;
    } else {
      assert(false);
    }
  } else {
    // Convert to any another format.

    // Currently only support COOR -> X.
    // If matrix is not COOR, convert to COOR.
    if (matrix.format != BSP_COOR) {
      bsp_matrix_t intermediate = bsp_convert_matrix(matrix, BSP_COOR);
      bsp_matrix_t result = bsp_convert_matrix(intermediate, format);
      bsp_destroy_matrix_t(intermediate);
      return result;
    } else {
      if (format == BSP_CSR) {
        // Convert COOR -> CSR

        bsp_matrix_t result = bsp_construct_default_matrix_t();

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

        result.values = bsp_copy_construct_array_t(matrix.values);

        if (index_type == matrix.indices_1.type) {
          result.indices_1 = bsp_copy_construct_array_t(matrix.indices_1);
        } else {
          result.indices_1 = bsp_construct_array_t(matrix.nnz, index_type);

          for (size_t i = 0; i < matrix.nnz; i++) {
            size_t index;
            bsp_array_read(matrix.indices_1, i, index);
            bsp_array_write(result.indices_1, i, index);
          }
        }

        result.pointers_to_1 =
            bsp_construct_array_t(matrix.nrows + 1, index_type);

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
      } else {
        assert(false);
      }
    }
  }
}
