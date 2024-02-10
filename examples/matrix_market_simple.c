#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  bsp_mm_metadata m = bsp_mmread_metadata("chesapeake/chesapeake.mtx");

  printf("%lu x %lu matrix with %lu nonzeros.\n", m.nrows, m.ncols, m.nnz);
  printf(
      "Matrix Market format is \"%s\" with type \"%s\" and structure \"%s\"\n",
      m.format, m.type, m.structure);

  printf("File has comments:\n\"%s\"\n", m.comments);

  bsp_matrix_t matrix =
      bsp_mmread("chesapeake/chesapeake.mtx", BSP_BINT8, BSP_UINT64);

  int8_t* values = matrix.values.data;
  uint64_t* rowind = matrix.indices_0.data;
  uint64_t* colind = matrix.indices_1.data;

  for (size_t i = 0; i < matrix.nnz; i++) {
    printf("%llu, %llu: %d\n", rowind[i], colind[i], values[i]);
  }

  return 0;
}
