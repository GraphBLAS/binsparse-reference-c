#pragma once

#include <assert.h>
#include <binsparse/matrix.h>
#include <cJSON/cJSON.h>

bsp_matrix_t bsp_read_matrix(char* file_name) {
  hid_t f = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);

  bsp_matrix_t matrix = bsp_construct_default_matrix_t();

  char* json_string = bsp_read_attribute(f, "binsparse");

  cJSON* j = cJSON_Parse(json_string);

  assert(j != NULL);
  assert(cJSON_IsObject(j));

  cJSON* binsparse = cJSON_GetObjectItemCaseSensitive(j, "binsparse");
  assert(cJSON_IsObject(binsparse));

  cJSON* version_ = cJSON_GetObjectItemCaseSensitive(binsparse, "version");

  assert(version_ != NULL);

  assert(cJSON_IsNumber(version_));

  double version = cJSON_GetNumberValue(version_);
  assert(version >= 1.0);

  cJSON* format_ = cJSON_GetObjectItemCaseSensitive(binsparse, "format");
  assert(format_ != NULL);
  char* format_string = cJSON_GetStringValue(format_);

  bsp_matrix_format_t format = bsp_get_matrix_format(format_string);

  matrix.format = format;

  cJSON* nnz_ = cJSON_GetObjectItemCaseSensitive(binsparse, "nnz");
  assert(nnz_ != NULL);
  size_t nnz = cJSON_GetNumberValue(nnz_);

  matrix.nnz = nnz;

  cJSON* shape_ = cJSON_GetObjectItemCaseSensitive(binsparse, "shape");
  assert(shape_ != NULL);

  assert(cJSON_GetArraySize(shape_) == 2);

  cJSON* nrows_ = cJSON_GetArrayItem(shape_, 0);
  assert(nrows_ != NULL);

  size_t nrows = cJSON_GetNumberValue(nrows_);

  cJSON* ncols_ = cJSON_GetArrayItem(shape_, 1);
  assert(ncols_ != NULL);

  size_t ncols = cJSON_GetNumberValue(ncols_);

  matrix.nrows = nrows;
  matrix.ncols = ncols;
  matrix.nnz = nnz;
  matrix.format = format;

  cJSON* data_types_ =
      cJSON_GetObjectItemCaseSensitive(binsparse, "data_types");
  assert(data_types_ != NULL);

  if (cJSON_HasObjectItem(data_types_, "values")) {
    matrix.values = bsp_read_array(f, "values");
  }

  if (cJSON_HasObjectItem(data_types_, "indices_0")) {
    matrix.indices_0 = bsp_read_array(f, "indices_0");
  }

  if (cJSON_HasObjectItem(data_types_, "indices_1")) {
    matrix.indices_1 = bsp_read_array(f, "indices_1");
  }

  if (cJSON_HasObjectItem(data_types_, "pointers_to_1")) {
    matrix.pointers_to_1 = bsp_read_array(f, "pointers_to_1");
  }

  if (cJSON_HasObjectItem(binsparse, "structure")) {
    cJSON* structure_ =
        cJSON_GetObjectItemCaseSensitive(binsparse, "structure");
    char* structure = cJSON_GetStringValue(structure_);
    matrix.structure = bsp_get_structure(structure);
  }

  cJSON_Delete(j);
  free(json_string);

  H5Fclose(f);
  return matrix;
}
