#pragma once

#include <H5.h>
#include <array.h>

// Write an array to a dataset / file
// Returns 0 on success, nonzero on error.
int bsp_write_array(hid_t f, char* label, bsp_array_t array) {

  hid_t hdf5_standard_type = bsp_hdf5_standard_type(array.type);
  hid_t fspace = H5Screate_simple(1, (hsize_t[]){array.size}, NULL)
  hid_t lcpl = H5Pcreate(H5P_LINK_CREATE);

  hid_t dset = H5Dcreate2(f, label, hdf5_standard_type, fspace, lcpl,
                          H5P_DEFAULT, H5P_DEFAULT);

  if (dset == H5I_INVALID_HID) {
    return -1;
  }

  hid_t hdf5_native_type = bsp_hdf5_native_type(array.type);

  H5Dwrite(dset, hdf5_native_type, H5S_ALL, fspace, H5P_DEFAULT, array.data);

  H5Sclose(fspace);
  H5Pclose(lcpl);

  return 0;
}