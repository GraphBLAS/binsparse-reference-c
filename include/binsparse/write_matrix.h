#pragma once

#include <assert.h>
#include <binsparse/matrix.h>
#include <cJSON/cJSON.h>
#include <unistd.h>

char* bsp_generate_json(bsp_matrix_t matrix, cJSON* user_json) {
  cJSON* j = cJSON_CreateObject();
  assert(j != NULL);

  cJSON* binsparse = cJSON_CreateObject();

  assert(binsparse != NULL);

  cJSON_AddItemToObject(j, "binsparse", binsparse);

  cJSON* item;
  cJSON_ArrayForEach(item, user_json) {
    cJSON_AddItemToObject(j, item->string, item);
  }

  cJSON_AddStringToObject(binsparse, "version", BINSPARSE_VERSION);

  cJSON_AddStringToObject(binsparse, "format",
                          bsp_get_matrix_format_string(matrix.format));

  cJSON* shape = cJSON_AddArrayToObject(binsparse, "shape");

  cJSON* nrows = cJSON_CreateNumber(matrix.nrows);
  cJSON* ncols = cJSON_CreateNumber(matrix.ncols);

  cJSON_AddItemToArray(shape, nrows);
  cJSON_AddItemToArray(shape, ncols);

  cJSON_AddNumberToObject(binsparse, "number_of_stored_values", matrix.nnz);

  cJSON* data_types = cJSON_AddObjectToObject(binsparse, "data_types");

  if (!matrix.is_iso) {
    cJSON_AddStringToObject(data_types, "values",
                            bsp_get_type_string(matrix.values.type));
  } else {
    char* base_type_string = bsp_get_type_string(matrix.values.type);
    size_t len = strlen(base_type_string) + 6;
    char* type_string = (char*) malloc(sizeof(char) * len);

    strncpy(type_string, "iso[", len);
    strncpy(type_string + 4, base_type_string, len - 4);
    strncpy(type_string + len - 2, "]", 2);

    cJSON_AddStringToObject(data_types, "values", type_string);

    free(type_string);
  }

  if (matrix.indices_0.data != NULL) {
    cJSON_AddStringToObject(data_types, "indices_0",
                            bsp_get_type_string(matrix.indices_0.type));
  }

  if (matrix.indices_1.data != NULL) {
    cJSON_AddStringToObject(data_types, "indices_1",
                            bsp_get_type_string(matrix.indices_1.type));
  }

  if (matrix.pointers_to_1.data != NULL) {
    cJSON_AddStringToObject(data_types, "pointers_to_1",
                            bsp_get_type_string(matrix.pointers_to_1.type));
  }

  if (matrix.structure != BSP_GENERAL) {
    cJSON_AddStringToObject(binsparse, "structure",
                            bsp_get_structure_string(matrix.structure));
  }

  char* string = cJSON_Print(j);

  cJSON_Delete(j);

  return string;
}

int bsp_write_matrix_to_group(hid_t f, bsp_matrix_t matrix, cJSON* user_json) {
  int result = bsp_write_array(f, "values", matrix.values);

  if (result != 0)
    return result;

  if (matrix.indices_0.size > 0) {
    result = bsp_write_array(f, "indices_0", matrix.indices_0);
    if (result != 0) {
      return result;
    }
  }

  if (matrix.indices_1.size > 0) {
    result = bsp_write_array(f, "indices_1", matrix.indices_1);
    if (result != 0) {
      return result;
    }
  }

  if (matrix.pointers_to_1.size > 0) {
    result = bsp_write_array(f, "pointers_to_1", matrix.pointers_to_1);
    if (result != 0) {
      return result;
    }
  }

  char* json_string = bsp_generate_json(matrix, user_json);

  bsp_write_attribute(f, "binsparse", json_string);
  free(json_string);

  return 0;
}

int bsp_write_matrix(char* fname, bsp_matrix_t matrix, char* group,
                     cJSON* user_json) {
  if (group == NULL) {
    hid_t f = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    bsp_write_matrix_to_group(f, matrix, user_json);
    H5Fclose(f);
  } else {
    hid_t f;
    if (access(fname, F_OK) == 0) {
      f = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);
    } else {
      f = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    hid_t g = H5Gcreate1(f, group, H5P_DEFAULT);
    bsp_write_matrix_to_group(g, matrix, user_json);
    H5Gclose(g);
    H5Fclose(f);
  }
  return 0;
}
