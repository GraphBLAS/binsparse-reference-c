#include <binsparse/binsparse.h>
#include <stdio.h>

int main(int argc, char** argv) {

  if (argc < 3) {
    printf("usage: ./mtx2bsp [inputfile_name.mtx] [outputfile_name.bsp.hdf5] "
           "[optional: dataset]\n");
    return 1;
  }

  char* input_fname = argv[1];
  char* output_fname = argv[2];

  char* group_name = NULL;

  if (argc >= 4) {
    group_name = argv[3];
  }

  bsp_mm_metadata m = bsp_mmread_metadata(input_fname);

  printf("%lu x %lu matrix with %lu nonzeros.\n", m.nrows, m.ncols, m.nnz);
  printf(
      "Matrix Market format is \"%s\" with type \"%s\" and structure \"%s\"\n",
      m.format, m.type, m.structure);

  if (strlen(m.comments) < 100) {
    printf("File has comments:\n\"%s\"\n", m.comments);
  } else {
    printf("File has very long comments, not printing.\n");
  }

  printf(" === Reading file... ===\n");
  bsp_matrix_t matrix = bsp_mmread(input_fname);
  printf(" === Done reading. ===\n");

  bsp_print_matrix_info(matrix);

  printf(" === Writing to %s... ===\n", output_fname);
  bsp_write_matrix(output_fname, matrix, group_name);
  printf(" === Done writing. ===\n");

  bsp_destroy_matrix_t(matrix);

  return 0;
}
