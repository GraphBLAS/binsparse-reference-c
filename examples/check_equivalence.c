#include <binsparse/binsparse.h>

int check_array_equivalence(bsp_array_t array1, bsp_array_t array2) {
  if (array1.size != array2.size) {
    fprintf(stderr, "Array sizes do not match. %zu != %zu\n", array1.size,
            array2.size);
    return 1;
  }

  if (array1.size == 0) {
    return 0;
  }

  bsp_matrix_market_type_t mm_type1 = BSP_MM_REAL;

  if ((array1.type >= BSP_UINT8 && array1.type <= BSP_INT64) ||
      array1.type == BSP_BINT8) {
    mm_type1 = BSP_MM_INTEGER;
  } else if (array1.type >= BSP_FLOAT32 && array1.type <= BSP_FLOAT64) {
    mm_type1 = BSP_MM_REAL;
  } else if (array1.type == BSP_COMPLEX_FLOAT32 ||
             array1.type == BSP_COMPLEX_FLOAT64) {
    mm_type1 = BSP_MM_COMPLEX;
  } else {
    fprintf(stderr, "Unhandled array type.\n");
    return 2;
  }

  bsp_matrix_market_type_t mm_type2 = BSP_MM_REAL;

  if ((array2.type >= BSP_UINT8 && array2.type <= BSP_INT64) ||
      array2.type == BSP_BINT8) {
    mm_type2 = BSP_MM_INTEGER;
  } else if (array2.type >= BSP_FLOAT32 && array2.type <= BSP_FLOAT64) {
    mm_type2 = BSP_MM_REAL;
  } else if (array2.type == BSP_COMPLEX_FLOAT32 ||
             array2.type == BSP_COMPLEX_FLOAT64) {
    mm_type2 = BSP_MM_COMPLEX;
  } else {
    fprintf(stderr, "Unhandled array type.\n");
    return 2;
  }

  if (mm_type1 != mm_type2) {
    fprintf(stderr, "Array types do not match.\n");
    return 3;
  }

  for (size_t i = 0; i < array1.size; i++) {
    if (mm_type1 == BSP_MM_INTEGER) {
      size_t value1, value2;
      bsp_array_read(array1, i, value1);
      bsp_array_read(array2, i, value2);

      if (value1 != value2) {
        fprintf(stderr, "Array values are not equal.\n");
        return 4;
      }
    } else if (mm_type1 == BSP_MM_REAL) {
      double value1, value2;
      bsp_array_read(array1, i, value1);
      bsp_array_read(array2, i, value2);

      if (value1 != value2) {
        fprintf(stderr, "Array values are not equal.\n");
        return 4;
      }
    } else if (mm_type1 == BSP_MM_COMPLEX) {
      double _Complex value1, value2;
      bsp_array_read(array1, i, value1);
      bsp_array_read(array2, i, value2);

      if (value1 != value2) {
        fprintf(stderr, "Array values are not equal.\n");
        return 4;
      }
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf(
        "usage: ./check_equivalence [file1.{mtx/hdf5}] [file2.{mtx/hdf5}]\n");
    return 1;
  }

  char* file1 = argv[1];
  char* file2 = argv[2];

  bsp_fdataset_info_t info1 = bsp_parse_fdataset_string(argv[1]);
  bsp_fdataset_info_t info2 = bsp_parse_fdataset_string(argv[2]);

  printf("Matrix 1: %s and %s\n", info1.fname,
         (info1.dataset == NULL) ? "root" : info1.dataset);
  printf("Matrix 2: %s and %s\n", info2.fname,
         (info2.dataset == NULL) ? "root" : info2.dataset);

  bsp_matrix_t matrix1 = bsp_read_matrix(info1.fname, info1.dataset);
  bsp_matrix_t matrix2 = bsp_read_matrix(info2.fname, info2.dataset);

  // If matrices are not the same format, try to convert.
  if (matrix1.format != matrix2.format) {
    if (matrix1.format != BSP_COOR) {
      bsp_matrix_t intermediate = bsp_convert_matrix(matrix1, BSP_COOR);
      bsp_destroy_matrix_t(matrix1);
      matrix1 = intermediate;
    }

    if (matrix2.format != BSP_COOR) {
      bsp_matrix_t intermediate = bsp_convert_matrix(matrix2, BSP_COOR);
      bsp_destroy_matrix_t(matrix2);
      matrix2 = intermediate;
    }
  }

  if (matrix1.format != matrix2.format) {
    fprintf(stderr, "Formats do not match. (%s != %s)\n",
            bsp_get_matrix_format_string(matrix1.format),
            bsp_get_matrix_format_string(matrix2.format));
    fprintf(stderr, "FAIL!\n");
    return 1;
  }

  if (matrix1.structure != matrix2.structure) {
    fprintf(stderr, "Structures do not match. (%s != %s)\n",
            bsp_get_structure_string(matrix1.structure),
            bsp_get_structure_string(matrix2.structure));
    fprintf(stderr, "FAIL!\n");
    return 2;
  }

  if (matrix1.nrows != matrix2.nrows || matrix1.ncols != matrix2.ncols) {
    fprintf(stderr, "Dimensions do not match. (%zu, %zu) != (%zu, %zu)\n",
            matrix1.nrows, matrix1.ncols, matrix2.nrows, matrix2.ncols);
    fprintf(stderr, "FAIL!\n");
    return 3;
  }

  if (matrix1.nnz != matrix2.nnz) {
    fprintf(stderr, "Number of stored values does not match. %zu != %zu\n",
            matrix1.nnz, matrix2.nnz);
    fprintf(stderr, "FAIL!\n");
    return 4;
  }

  if (matrix1.is_iso != matrix2.is_iso) {
    fprintf(stderr, "ISO-ness does not match. %s != %s\n",
            (matrix1.is_iso) ? "true" : "false",
            (matrix2.is_iso) ? "true" : "false");
    fprintf(stderr, "FAIL!\n");
    return 5;
  }

  if (check_array_equivalence(matrix1.values, matrix2.values) != 0) {
    fprintf(stderr, "Value arrays not equivalent.\n");
    fprintf(stderr, "FAIL!\n");
    return 6;
  }

  if (check_array_equivalence(matrix1.indices_0, matrix2.indices_0) != 0) {
    fprintf(stderr, "Indices_0 arrays not equivalent.\n");
    fprintf(stderr, "FAIL!\n");
    return 7;
  }

  if (check_array_equivalence(matrix1.indices_1, matrix2.indices_1) != 0) {
    fprintf(stderr, "Indices_1 arrays not equivalent.\n");
    fprintf(stderr, "FAIL!\n");
    return 8;
  }

  if (check_array_equivalence(matrix1.pointers_to_1, matrix2.pointers_to_1) !=
      0) {
    fprintf(stderr, "Pointers_to_1 arrays not equivalent.\n");
    fprintf(stderr, "FAIL!\n");
    return 9;
  }

  printf("The files are equivalent.\n");
  printf("OK!\n");

  return 0;
}
