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
    assert(false);
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

        // TODO: consider whether to produce files with varying integer types
        //       for row indices, column indices, and offsets.

        size_t max_dim =
            (matrix.nrows > matrix.ncols) ? matrix.nrows : matrix.ncols;

        size_t max_value =
            (max_dim > matrix.values.size) ? max_dim : matrix.values.size;

        bsp_type_t value_type = matrix.values.type;
        bsp_type_t index_type = bsp_pick_integer_type(max_value);

        result.values = bsp_construct_array_t(matrix.nnz, value_type);
        result.indices_0 = bsp_construct_array_t(matrix.nnz, index_type);
        result.pointers_to_1 =
            bsp_construct_array_t(matrix.nrows + 1, index_type);

        bsp_array_t values = result.values;
        bsp_array_t colind = result.indices_0;
        bsp_array_t rowptr = result.pointers_to_1;

        bsp_array_write(rowptr, 0, 0);

        size_t r = 0;
        size_t c = 0;
        for (size_t c = 0; c < matrix.nnz; c++) {
          bsp_array_awrite(values, c, matrix.values, c);
          bsp_array_awrite(colind, c, matrix.indices_1, c);

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
