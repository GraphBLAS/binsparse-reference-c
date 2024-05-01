#include <binsparse/binsparse.h>

herr_t visit_group(hid_t loc_id, const char* name, const H5L_info_t* linfo,
                   void* opdata);

void print_group_info(hid_t g, const char* name) {
  hid_t bsp_json;

  H5E_BEGIN_TRY {
    bsp_json = H5Aopen(g, "binsparse", H5P_DEFAULT);
  }
  H5E_END_TRY;

  if (bsp_json != H5I_INVALID_HID) {
    char* json_string = bsp_read_attribute(g, "binsparse");

    cJSON* j = cJSON_Parse(json_string);

    assert(j != NULL);
    assert(cJSON_IsObject(j));

    cJSON* binsparse = cJSON_GetObjectItemCaseSensitive(j, "binsparse");
    assert(cJSON_IsObject(binsparse));

    cJSON* version_ = cJSON_GetObjectItemCaseSensitive(binsparse, "version");

    assert(version_ != NULL);

    assert(cJSON_IsString(version_));
    char* version_string = cJSON_GetStringValue(version_);

    // TODO: check version.

    cJSON* format_ = cJSON_GetObjectItemCaseSensitive(binsparse, "format");
    assert(format_ != NULL);
    char* format_string = cJSON_GetStringValue(format_);

    cJSON* nnz_ = cJSON_GetObjectItemCaseSensitive(binsparse, "nnz");
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

    char full_group_path[2048];
    size_t size = H5Iget_name(g, full_group_path, 2048);

    printf("Group \"%s\": Version %s Binsparse matrix. Format %s, %zu x %zu.\n",
           full_group_path, version_string, format_string, nrows, ncols);
  }

  H5Literate(g, H5_INDEX_NAME, H5_ITER_INC, NULL, visit_group, NULL);
}

herr_t visit_group(hid_t loc_id, const char* name, const H5L_info_t* linfo,
                   void* opdata) {
  hid_t g = H5Oopen(loc_id, name, H5P_DEFAULT);
  H5I_type_t type = H5Iget_type(g);
  if (type == H5I_GROUP) {
    print_group_info(g, name);
  }
  H5Oclose(g);
  return 0;
}

int main(int argc, char** argv) {

  if (argc < 2) {
    printf("usage: ./bsp-ls [file_name.mtx]\n");
    return 1;
  }

  char* fname = argv[1];

  hid_t f = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);

  print_group_info(f, "/");

  H5Fclose(f);

  return 0;
}
