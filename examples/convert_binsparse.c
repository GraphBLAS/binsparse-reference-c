#include <binsparse/binsparse.h>
#include <stdio.h>

int main(int argc, char** argv) {

  if (argc < 2) {
    printf("usage: ./convert_binsparse [file_name.mtx]\n");
    return 1;
  }

  bsp_mm_metadata m = bsp_mmread_metadata("chesapeake/chesapeake.mtx");

  printf("%lu x %lu matrix with %lu nonzeros.\n", m.nrows, m.ncols, m.nnz);
  printf(
      "Matrix Market format is \"%s\" with type \"%s\" and structure \"%s\"\n",
      m.format, m.type, m.structure);

  printf("File has comments:\n\"%s\"\n", m.comments);

  printf(" === Reading file... ===\n");
  bsp_matrix_t matrix = bsp_mmread("chesapeake/chesapeake.mtx");
  printf(" === Read file. ===\n");

  bsp_print_matrix_info(matrix);

  int8_t* values = matrix.values.data;
  uint8_t* rowind = matrix.indices_0.data;
  uint8_t* colind = matrix.indices_1.data;

  for (size_t i = 0; i < matrix.nnz; i++) {
    printf("%llu, %llu: %d\n", rowind[i], colind[i], values[i]);
  }

  return 0;
}
