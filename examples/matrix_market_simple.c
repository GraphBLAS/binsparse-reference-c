#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  bsp_mm_metadata m = bsp_mmread_metadata("chesapeake/chesapeake.mtx");

  printf("%lu x %lu matrix with %lu nonzeros.\n", m.nrows, m.ncols, m.nnz);
  printf("Matrix Market format is \"%s\" with type \"%s\" and structure \"%s\"\n",
         m.format, m.type, m.structure);

  printf("File has comments:\n\"%s\"\n", m.comments);

  return 0;
}