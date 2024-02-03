#pragma once

#include <binsparse/array.h>
#include <hdf5.h>

// Write an array to a dataset / file
// Returns 0 on success, nonzero on error.
int bsp_write_array(hid_t f, char* label, bsp_array_t array) {
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

  herr_t status =
      H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.data);

  if (status < 0) {
    return bsp_construct_default_array_t();
  }

  H5Dclose(dset);
  H5Sclose(fspace);
  return array;
}

void bsp_write_attribute(hid_t f, char* label, char* string) {
  hid_t strtype = H5Tcopy(H5T_C_S1);
  H5Tset_size(strtype, H5T_VARIABLE);
  H5Tset_cset(strtype, H5T_CSET_UTF8);
  hid_t dataspace = H5Screate(H5S_SCALAR);

  hid_t attribute =
      H5Acreate2(f, label, strtype, dataspace, H5P_DEFAULT, H5P_DEFAULT);

  H5Awrite(attribute, strtype, &string);

  H5Aclose(attribute);
  H5Sclose(dataspace);
  H5Tclose(strtype);
}
