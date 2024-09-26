// SPDX-License-Identifier: BSD-3-Clause

#include <binsparse/binsparse.h>
#include <stdio.h>

#include <time.h>

double gettime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return ((double) time.tv_sec) + ((double) 1e-9) * time.tv_nsec;
}

int main(int argc, char** argv) {

  if (argc < 3) {
    printf("usage: ./mtx2bsp [input.mtx] [output.bsp.h5]:[optional: group] "
           "[optional: format] [optional: compression level 0-9]\n");
    printf("\n");
    printf("Description: Convert a Matrix Market file to a Binsparse HDF5 "
           "file.\n");
    printf("             Users can optionally provide an HDF5 group to store "
           "the\n");
    printf("             file in as well as a specific format. The default "
           "format\n");
    printf("             is row-sorted COO (COOR).\n");
    printf("\n");
    printf("example: ./mtx2bsp chesapeake.mtx chesapeake.bsp.h5\n");
    printf("        - Convert Matrix Market file `chesapeake.mtx` to Binsparse "
           "HDF5 file `chesapeake.bsp.h5`.\n");
    printf("        - Matrix will be stored in root group.\n");
    printf("        - Matrix will be stored in COOR format.\n");
    printf("\n");
    printf("example: ./mtx2bsp chesapeake.mtx chesapeake.bsp.h5:chesapeake\n");
    printf("         - Same as previous example, but matrix will be stored in "
           "HDF5 group `chesapeake`.\n");
    printf("\n");
    printf(
        "example: ./mtx2bsp chesapeake.mtx chesapeake.bsp.h5:chesapeake CSR\n");
    printf("         - Same as previous example, but matrix will use CSR "
           "format.\n");
    printf("example: ./mtx2bsp chesapeake.mtx chesapeake.bsp.h5:chesapeake CSR "
           "5\n");
    printf("         - Same as previous example, but will use GZip compression "
           "level 5.\n");
    printf("           0 is no compression, 1-9 correspond to GZip compression "
           "levels.\n");
    printf("           Default is 9.\n");
    return 1;
  }

  // NOTE: this binary automatically performs "declamping" of floating point
  //       values greater/less than 1e308/-1e308 in order to restore inf values
  //       in SuiteSparse Matrix Collection matrices.
  bool perform_suitesparse_declamping = true;

  char* input_fname = argv[1];

  bsp_fdataset_info_t info2 = bsp_parse_fdataset_string(argv[2]);
  char* output_fname = info2.fname;
  char* group_name = info2.dataset;

  char* format_name = NULL;

  if (argc >= 4) {
    format_name = argv[3];
  }

  int compression_level = 9;

  if (argc >= 5) {
    compression_level = atoi(argv[4]);
  }

  const char* input_file_extension = bsp_get_file_extension(input_fname);
  const char* output_file_extension = bsp_get_file_extension(output_fname);

  if (input_file_extension == NULL ||
      strcmp(input_file_extension, ".mtx") != 0) {
    fprintf(stderr,
            "error: input file \"%s\" is not a Matrix Market file. "
            "(Its extension is not '.mtx'.)\n",
            input_fname);
    return 1;
  }

  if (output_file_extension == NULL ||
      (strcmp(output_file_extension, ".h5") != 0 &&
       strcmp(output_file_extension, ".hdf5") != 0)) {
    fprintf(stderr,
            "error: output file \"%s\" is not an HDF5 file. "
            "(Its extension is not '.h5' or '.hdf5'.)\n",
            output_fname);
    return 1;
  }

  bsp_mm_metadata m = bsp_mmread_metadata(input_fname);

  bsp_matrix_format_t format = BSP_COOR;
  if (format_name != NULL) {
    format = bsp_get_matrix_format(format_name);
    assert(format != 0);
  }

  printf("%lu x %lu matrix with %lu nonzeros.\n", m.nrows, m.ncols, m.nnz);
  printf(
      "Matrix Market format is \"%s\" with type \"%s\" and structure \"%s\"\n",
      m.format, m.type, m.structure);

  if (strlen(m.comments) < 100) {
    printf("File has comments:\n\"%s\"\n", m.comments);
  } else {
    printf("File has very long comments, not printing.\n");
  }

  printf("Printing with compression level %d.\n", compression_level);

  cJSON* user_json = cJSON_CreateObject();

  assert(user_json != NULL);

  cJSON_AddStringToObject(user_json, "comment", m.comments);

  printf(" === Reading file... ===\n");
  double begin = gettime();
  bsp_matrix_t matrix = bsp_mmread(input_fname);
  double end = gettime();
  printf(" === Done reading. ===\n");

  double duration = end - begin;
  printf("%lf seconds reading Matrix Market file...\n", duration);

  if (perform_suitesparse_declamping) {
    begin = gettime();
    bsp_matrix_declamp_values(matrix);
    end = gettime();
    duration = end - begin;
    printf("%lf seconds declamping...\n", duration);
  }

  begin = gettime();
  matrix = bsp_matrix_minimize_values(matrix);
  end = gettime();
  duration = end - begin;
  printf("%lf seconds minimizing values...\n", duration);

  if (format != BSP_COOR) {
    begin = gettime();
    bsp_matrix_t converted_matrix = bsp_convert_matrix(matrix, format);
    bsp_destroy_matrix_t(matrix);
    matrix = converted_matrix;
    end = gettime();
    duration = end - begin;
    printf("%lf seconds converting to %s format...\n", duration,
           bsp_get_matrix_format_string(format));
  }

  bsp_print_matrix_info(matrix);

  printf(" === Writing to %s... ===\n", output_fname);
  begin = gettime();
  bsp_write_matrix(output_fname, matrix, group_name, user_json,
                   compression_level);
  end = gettime();
  duration = end - begin;
  printf("%lf seconds writing Binsparse file...\n", duration);
  printf(" === Done writing. ===\n");

  bsp_destroy_matrix_t(matrix);

  return 0;
}
