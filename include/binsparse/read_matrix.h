/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <assert.h>
#include <binsparse/matrix.h>
#include <cJSON/cJSON.h>

bsp_matrix_t bsp_read_matrix_from_group_parallel(hid_t f, int num_threads) {
  bsp_matrix_t matrix = bsp_construct_default_matrix_t();

  char* json_string = bsp_read_attribute(f, (char*) "binsparse");

  cJSON* j = cJSON_Parse(json_string);

  assert(j != NULL);
  assert(cJSON_IsObject(j));

  cJSON* binsparse = cJSON_GetObjectItemCaseSensitive(j, "binsparse");
  assert(cJSON_IsObject(binsparse));

  cJSON* version_ = cJSON_GetObjectItemCaseSensitive(binsparse, "version");

  assert(version_ != NULL);

  assert(cJSON_IsString(version_));

  // TODO: check version.

  cJSON* format_ = cJSON_GetObjectItemCaseSensitive(binsparse, "format");
  assert(format_ != NULL);
  char* format_string = cJSON_GetStringValue(format_);

  bsp_matrix_format_t format = bsp_get_matrix_format(format_string);

  assert(format != 0);

  matrix.format = format;

  cJSON* nnz_ =
      cJSON_GetObjectItemCaseSensitive(binsparse, "number_of_stored_values");
  assert(nnz_ != NULL);
  size_t nnz = cJSON_GetNumberValue(nnz_);

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
    matrix.values = bsp_read_array_parallel(f, (char*) "values", num_threads);

    cJSON* value_type = cJSON_GetObjectItemCaseSensitive(data_types_, "values");
    char* type_string = cJSON_GetStringValue(value_type);

    if (strlen(type_string) >= 4 && strncmp(type_string, "iso[", 4) == 0) {
      matrix.is_iso = true;
      type_string += 4;
    }

    if (strlen(type_string) >= 8 && strncmp(type_string, "complex[", 8) == 0) {
      matrix.values = bsp_fp_array_to_complex(matrix.values);
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_0")) {
    matrix.indices_0 =
        bsp_read_array_parallel(f, (char*) "indices_0", num_threads);
  }

  if (cJSON_HasObjectItem(data_types_, "indices_1")) {
    matrix.indices_1 =
        bsp_read_array_parallel(f, (char*) "indices_1", num_threads);
  }

  if (cJSON_HasObjectItem(data_types_, "pointers_to_1")) {
    matrix.pointers_to_1 =
        bsp_read_array_parallel(f, (char*) "pointers_to_1", num_threads);
  }

  if (cJSON_HasObjectItem(binsparse, "structure")) {
    cJSON* structure_ =
        cJSON_GetObjectItemCaseSensitive(binsparse, "structure");
    char* structure = cJSON_GetStringValue(structure_);
    matrix.structure = bsp_get_structure(structure);
  }

  cJSON_Delete(j);
  free(json_string);

  return matrix;
}

bsp_matrix_t bsp_read_matrix_from_group(hid_t f) {
  bsp_matrix_t matrix = bsp_construct_default_matrix_t();

  char* json_string = bsp_read_attribute(f, (char*) "binsparse");

  cJSON* j = cJSON_Parse(json_string);

  assert(j != NULL);
  assert(cJSON_IsObject(j));

  cJSON* binsparse = cJSON_GetObjectItemCaseSensitive(j, "binsparse");
  assert(cJSON_IsObject(binsparse));

  cJSON* version_ = cJSON_GetObjectItemCaseSensitive(binsparse, "version");

  assert(version_ != NULL);

  assert(cJSON_IsString(version_));

  // TODO: check version.

  cJSON* format_ = cJSON_GetObjectItemCaseSensitive(binsparse, "format");
  assert(format_ != NULL);
  char* format_string = cJSON_GetStringValue(format_);

  bsp_matrix_format_t format = bsp_get_matrix_format(format_string);

  assert(format != 0);

  matrix.format = format;

  cJSON* nnz_ =
      cJSON_GetObjectItemCaseSensitive(binsparse, "number_of_stored_values");
  assert(nnz_ != NULL);
  size_t nnz = cJSON_GetNumberValue(nnz_);

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
    matrix.values = bsp_read_array(f, (char*) "values");

    cJSON* value_type = cJSON_GetObjectItemCaseSensitive(data_types_, "values");
    char* type_string = cJSON_GetStringValue(value_type);

    if (strlen(type_string) >= 4 && strncmp(type_string, "iso[", 4) == 0) {
      matrix.is_iso = true;
      type_string += 4;
    }

    if (strlen(type_string) >= 8 && strncmp(type_string, "complex[", 8) == 0) {
      matrix.values = bsp_fp_array_to_complex(matrix.values);
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_0")) {
    matrix.indices_0 = bsp_read_array(f, (char*) "indices_0");
  }

  if (cJSON_HasObjectItem(data_types_, "indices_1")) {
    matrix.indices_1 = bsp_read_array(f, (char*) "indices_1");
  }

  if (cJSON_HasObjectItem(data_types_, "pointers_to_1")) {
    matrix.pointers_to_1 = bsp_read_array(f, (char*) "pointers_to_1");
  }

  if (cJSON_HasObjectItem(binsparse, "structure")) {
    cJSON* structure_ =
        cJSON_GetObjectItemCaseSensitive(binsparse, "structure");
    char* structure = cJSON_GetStringValue(structure_);
    matrix.structure = bsp_get_structure(structure);
  }

  cJSON_Delete(j);
  free(json_string);

  return matrix;
}

size_t bsp_final_dot(const char* str) {
  size_t dot_idx = 0;
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (str[i] == '.') {
      dot_idx = i;
    }
  }
  return dot_idx;
}

bsp_matrix_t bsp_read_matrix_parallel(const char* file_name, const char* group,
                                      int num_threads) {
  if (group == NULL) {
    size_t idx = bsp_final_dot(file_name);
    if (strcmp(file_name + idx, ".hdf5") == 0 ||
        strcmp(file_name + idx, ".h5") == 0) {
      hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
      bsp_matrix_t matrix = bsp_read_matrix_from_group_parallel(f, num_threads);
      H5Fclose(f);
      return matrix;
    } else if (strcmp(file_name + idx, ".mtx") == 0) {
      return bsp_mmread(file_name);
    } else {
      assert(false);
    }
  } else {
    hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t g = H5Gopen1(f, group);
    bsp_matrix_t matrix = bsp_read_matrix_from_group_parallel(g, num_threads);
    H5Gclose(g);
    H5Fclose(f);
    return matrix;
  }
}

bsp_matrix_t bsp_read_matrix(const char* file_name, const char* group) {
  if (group == NULL) {
    size_t idx = bsp_final_dot(file_name);
    if (strcmp(file_name + idx, ".hdf5") == 0 ||
        strcmp(file_name + idx, ".h5") == 0) {
      hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
      bsp_matrix_t matrix = bsp_read_matrix_from_group(f);
      H5Fclose(f);
      return matrix;
    } else if (strcmp(file_name + idx, ".mtx") == 0) {
      return bsp_mmread(file_name);
    } else {
      assert(false);
    }
  } else {
    hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t g = H5Gopen1(f, group);
    bsp_matrix_t matrix = bsp_read_matrix_from_group(g);
    H5Gclose(g);
    H5Fclose(f);
    return matrix;
  }
}
