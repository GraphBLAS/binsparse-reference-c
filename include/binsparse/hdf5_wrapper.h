#pragma once

#include <assert.h>
#include <binsparse/array.h>
#include <hdf5.h>
#include <string.h>

// Write an array to a dataset / file
// Returns 0 on success, nonzero on error.
int bsp_write_array(hid_t f, char* label, bsp_array_t array) {
  if (array.type == BSP_COMPLEX_FLOAT32 || array.type == BSP_COMPLEX_FLOAT64) {
    array = bsp_complex_array_to_fp(array);
  }

  hid_t hdf5_standard_type = bsp_get_hdf5_standard_type(array.type);
  hid_t fspace = H5Screate_simple(1, (hsize_t[]){array.size}, NULL);
  hid_t lcpl = H5Pcreate(H5P_LINK_CREATE);

  hid_t dset = H5Dcreate2(f, label, hdf5_standard_type, fspace, lcpl,
                          H5P_DEFAULT, H5P_DEFAULT);

  if (dset == H5I_INVALID_HID) {
    return -1;
  }

  hid_t hdf5_native_type = bsp_get_hdf5_native_type(array.type);

  hid_t r = H5Dwrite(dset, hdf5_native_type, H5S_ALL, fspace, H5P_DEFAULT,
                     array.data);

  if (r == H5I_INVALID_HID) {
    return -2;
  }

  H5Sclose(fspace);
  H5Pclose(lcpl);

  return 0;
}

bsp_array_t bsp_read_array(hid_t f, char* label) {
  hid_t dset = H5Dopen2(f, label, H5P_DEFAULT);

  if (dset == H5I_INVALID_HID) {
    return bsp_construct_default_array_t();
  }

  hid_t fspace = H5Dget_space(dset);

  if (fspace == H5I_INVALID_HID) {
    return bsp_construct_default_array_t();
  }

  hsize_t dims[3];

  int r = H5Sget_simple_extent_dims(fspace, dims, NULL);

  if (r < 0) {
    return bsp_construct_default_array_t();
  }

  hid_t hdf5_type = H5Dget_type(dset);

  bsp_type_t type = bsp_get_bsp_type(hdf5_type);

  bsp_array_t array = bsp_construct_array_t(dims[0], type);

  herr_t status = H5Dread(dset, bsp_get_hdf5_native_type(type), H5S_ALL,
                          H5S_ALL, H5P_DEFAULT, array.data);

  if (status < 0) {
    return bsp_construct_default_array_t();
  }

  H5Dclose(dset);
  H5Sclose(fspace);
  return array;
}

void bsp_write_attribute(hid_t f, char* label, char* string) {
  hid_t strtype = H5Tcopy(H5T_C_S1);
  H5Tset_size(strtype, strlen(string));
  H5Tset_cset(strtype, H5T_CSET_UTF8);
  hsize_t size = 1;
  hid_t dataspace = H5Screate_simple(1, &size, H5P_DEFAULT);

  hid_t attribute =
      H5Acreate2(f, label, strtype, dataspace, H5P_DEFAULT, H5P_DEFAULT);

  H5Awrite(attribute, strtype, string);

  H5Tclose(strtype);
  H5Aclose(attribute);
  H5Sclose(dataspace);
}

char* bsp_read_attribute(hid_t f, char* label) {
  hid_t attribute = H5Aopen(f, label, H5P_DEFAULT);
  hid_t strtype = H5Aget_type(attribute);

  hid_t type_class = H5Tget_class(strtype);
  assert(type_class == H5T_STRING);

  size_t size = H5Tget_size(strtype);

  char* string = malloc(size + 1);

  H5Aread(attribute, strtype, string);

  H5Aclose(attribute);
  H5Tclose(strtype);

  return string;
}
