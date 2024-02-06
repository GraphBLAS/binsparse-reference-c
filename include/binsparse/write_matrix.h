#pragma once

#include <assert.h>
#include <binsparse/matrix.h>
#include <cJSON/cJSON.h>

char* bsp_generate_json(bsp_matrix_t matrix) {
  cJSON* j = cJSON_CreateObject();
  assert(j != NULL);

  cJSON* binsparse = cJSON_CreateObject();

  assert(binsparse != NULL);

  cJSON_AddItemToObject(j, "binsparse", binsparse);

  cJSON_AddNumberToObject(binsparse, "version", BINSPARSE_VERSION);

  cJSON_AddStringToObject(binsparse, "format",
                          bsp_get_matrix_format_string(matrix.format));

  cJSON* shape = cJSON_AddArrayToObject(binsparse, "shape");

  cJSON* nrows = cJSON_CreateNumber(matrix.nrows);
  cJSON* ncols = cJSON_CreateNumber(matrix.ncols);

  cJSON_AddItemToArray(shape, nrows);
  cJSON_AddItemToArray(shape, ncols);

  cJSON_AddNumberToObject(binsparse, "nnz", matrix.nnz);

  cJSON* data_types = cJSON_AddObjectToObject(binsparse, "data_types");

  cJSON_AddStringToObject(data_types, "values",
                          bsp_get_type_string(matrix.values.type));

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

int bsp_write_matrix(char* file_name, bsp_matrix_t matrix) {
  hid_t f = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

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

  char* json_string = bsp_generate_json(matrix);

  bsp_write_attribute(f, "binsparse", json_string);
  free(json_string);

  H5Fclose(f);
  return 0;
}
