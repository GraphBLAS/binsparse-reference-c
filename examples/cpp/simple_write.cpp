#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  const char* file_name = "test.hdf5";

  hid_t f = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  bsp_array_t array = bsp_construct_array_t(1000, BSP_INT32);

  int* values = (int*) array.data;

  for (size_t i = 0; i < array.size; i++) {
    bsp_array_write(array, i, i);
  }

  bsp_write_array(f, "test", array, 0);

  H5Fclose(f);
  bsp_destroy_array_t(array);

  return 0;
}
