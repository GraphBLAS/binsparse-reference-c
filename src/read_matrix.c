/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <binsparse/detail/allocator.h>
#include <binsparse/hdf5_wrapper.h>
#include <binsparse/matrix.h>
#include <binsparse/matrix_market/matrix_market_read.h>
#include <cJSON/cJSON.h>

#if __STDC_VERSION__ >= 201112L
bsp_error_t bsp_read_matrix_from_group_parallel(bsp_matrix_t* matrix, hid_t f,
                                                int num_threads) {
  bsp_construct_default_matrix_t(matrix);

  char* json_string;
  bsp_error_t error = bsp_read_attribute(&json_string, f, (char*) "binsparse");
  if (error != BSP_SUCCESS) {
    return error;
  }

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

  matrix->format = format;

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

  matrix->nrows = nrows;
  matrix->ncols = ncols;
  matrix->nnz = nnz;
  matrix->format = format;

  cJSON* data_types_ =
      cJSON_GetObjectItemCaseSensitive(binsparse, "data_types");
  assert(data_types_ != NULL);

  if (cJSON_HasObjectItem(data_types_, "values")) {
    error = bsp_read_array_parallel(&matrix->values, f, (char*) "values",
                                    num_threads);
    if (error != BSP_SUCCESS) {
      free(json_string);
      return error;
    }

    cJSON* value_type = cJSON_GetObjectItemCaseSensitive(data_types_, "values");
    char* type_string = cJSON_GetStringValue(value_type);

    if (strlen(type_string) >= 4 && strncmp(type_string, "iso[", 4) == 0) {
      matrix->is_iso = true;
      type_string += 4;
    }

    if (strlen(type_string) >= 8 && strncmp(type_string, "complex[", 8) == 0) {
      bsp_error_t error = bsp_fp_array_to_complex(&matrix->values);
      if (error != BSP_SUCCESS) {
        // TODO: handle error
        return error;
      }
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_0")) {
    error = bsp_read_array_parallel(&matrix->indices_0, f, (char*) "indices_0",
                                    num_threads);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      return error;
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_1")) {
    error = bsp_read_array_parallel(&matrix->indices_1, f, (char*) "indices_1",
                                    num_threads);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      bsp_destroy_array_t(&matrix->indices_0);
      return error;
    }
  }

  if (cJSON_HasObjectItem(data_types_, "pointers_to_1")) {
    error = bsp_read_array_parallel(&matrix->pointers_to_1, f,
                                    (char*) "pointers_to_1", num_threads);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      bsp_destroy_array_t(&matrix->indices_0);
      bsp_destroy_array_t(&matrix->indices_1);
      return error;
    }
  }

  if (cJSON_HasObjectItem(binsparse, "structure")) {
    cJSON* structure_ =
        cJSON_GetObjectItemCaseSensitive(binsparse, "structure");
    char* structure = cJSON_GetStringValue(structure_);
    matrix->structure = bsp_get_structure(structure);
  }

  cJSON_Delete(j);
  free(json_string);

  return BSP_SUCCESS;
}
#endif

bsp_error_t bsp_read_matrix_from_group_allocator(bsp_matrix_t* matrix, hid_t f,
                                                 bsp_allocator_t allocator) {
  bsp_construct_default_matrix_t_allocator(matrix, allocator);

  char* json_string;
  bsp_error_t error = bsp_read_attribute_allocator(
      &json_string, f, (char*) "binsparse", allocator);
  if (error != BSP_SUCCESS) {
    return error;
  }

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

  matrix->format = format;

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

  matrix->nrows = nrows;
  matrix->ncols = ncols;
  matrix->nnz = nnz;
  matrix->format = format;

  cJSON* data_types_ =
      cJSON_GetObjectItemCaseSensitive(binsparse, "data_types");
  assert(data_types_ != NULL);

  if (cJSON_HasObjectItem(data_types_, "values")) {
    error = bsp_read_array_allocator(&matrix->values, f, (char*) "values",
                                     allocator);
    if (error != BSP_SUCCESS) {
      free(json_string);
      return error;
    }

    cJSON* value_type = cJSON_GetObjectItemCaseSensitive(data_types_, "values");
    char* type_string = cJSON_GetStringValue(value_type);

    if (strlen(type_string) >= 4 && strncmp(type_string, "iso[", 4) == 0) {
      matrix->is_iso = true;
      type_string += 4;
    }

    if (strlen(type_string) >= 8 && strncmp(type_string, "complex[", 8) == 0) {
      bsp_error_t error = bsp_fp_array_to_complex(&matrix->values);
      if (error != BSP_SUCCESS) {
        // TODO: handle error
        return error;
      }
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_0")) {
    error = bsp_read_array_allocator(&matrix->indices_0, f, (char*) "indices_0",
                                     allocator);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      return error;
    }
  }

  if (cJSON_HasObjectItem(data_types_, "indices_1")) {
    error = bsp_read_array_allocator(&matrix->indices_1, f, (char*) "indices_1",
                                     allocator);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      bsp_destroy_array_t(&matrix->indices_0);
      return error;
    }
  }

  if (cJSON_HasObjectItem(data_types_, "pointers_to_1")) {
    error = bsp_read_array_allocator(&matrix->pointers_to_1, f,
                                     (char*) "pointers_to_1", allocator);
    if (error != BSP_SUCCESS) {
      free(json_string);
      bsp_destroy_array_t(&matrix->values);
      bsp_destroy_array_t(&matrix->indices_0);
      bsp_destroy_array_t(&matrix->indices_1);
      return error;
    }
  }

  if (cJSON_HasObjectItem(binsparse, "structure")) {
    cJSON* structure_ =
        cJSON_GetObjectItemCaseSensitive(binsparse, "structure");
    char* structure = cJSON_GetStringValue(structure_);
    matrix->structure = bsp_get_structure(structure);
  }

  cJSON_Delete(j);
  free(json_string);

  return BSP_SUCCESS;
}

bsp_error_t bsp_read_matrix_from_group(bsp_matrix_t* matrix, hid_t f) {
  return bsp_read_matrix_from_group_allocator(matrix, f, bsp_default_allocator);
}

static inline size_t bsp_final_dot(const char* str) {
  size_t dot_idx = 0;
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (str[i] == '.') {
      dot_idx = i;
    }
  }
  return dot_idx;
}

#if __STDC_VERSION__ >= 201112L
bsp_error_t bsp_read_matrix_parallel(bsp_matrix_t* matrix,
                                     const char* file_name, const char* group,
                                     int num_threads) {
  if (group == NULL) {
    size_t idx = bsp_final_dot(file_name);
    if (strcmp(file_name + idx, ".hdf5") == 0 ||
        strcmp(file_name + idx, ".h5") == 0) {
      hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
      bsp_error_t error =
          bsp_read_matrix_from_group_parallel(matrix, f, num_threads);
      H5Fclose(f);
      return error;
    } else if (strcmp(file_name + idx, ".mtx") == 0) {
      // TODO: implement error-handling for Matrix Market.
      *matrix = bsp_mmread(file_name);
      return BSP_SUCCESS;
    } else {
      return BSP_ERROR_IO;
    }
  } else {
    hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t g = H5Gopen1(f, group);
    bsp_error_t error =
        bsp_read_matrix_from_group_parallel(matrix, g, num_threads);
    H5Gclose(g);
    H5Fclose(f);
    return error;
  }
}
#endif

bsp_error_t bsp_read_matrix_allocator(bsp_matrix_t* matrix,
                                      const char* file_name, const char* group,
                                      bsp_allocator_t allocator) {
  if (group == NULL) {
    size_t idx = bsp_final_dot(file_name);
    if (strcmp(file_name + idx, ".hdf5") == 0 ||
        strcmp(file_name + idx, ".h5") == 0) {
      hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
      bsp_error_t error =
          bsp_read_matrix_from_group_allocator(matrix, f, allocator);
      H5Fclose(f);
      return error;
    } else if (strcmp(file_name + idx, ".mtx") == 0) {
      // TODO: implement error-handling for Matrix Market.
      *matrix = bsp_mmread(file_name);
      return BSP_SUCCESS;
    } else {
      return BSP_ERROR_IO;
    }
  } else {
    hid_t f = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t g = H5Gopen1(f, group);
    bsp_error_t error =
        bsp_read_matrix_from_group_allocator(matrix, g, allocator);
    H5Gclose(g);
    H5Fclose(f);
    return error;
  }
}

bsp_error_t bsp_read_matrix(bsp_matrix_t* matrix, const char* file_name,
                            const char* group) {
  return bsp_read_matrix_allocator(matrix, file_name, group,
                                   bsp_default_allocator);
}
