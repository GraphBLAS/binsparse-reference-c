#pragma once

#include <assert.h>
#include <binsparse/array.h>
#include <hdf5.h>
#include <string.h>

// Write an array to a dataset / file
// Returns 0 on success, nonzero on error.
int bsp_write_array(hid_t f, const char* label, bsp_array_t array,
                    int compression_level) {
  if (array.type == BSP_COMPLEX_FLOAT32 || array.type == BSP_COMPLEX_FLOAT64) {
    array = bsp_complex_array_to_fp(array);
  }

  hsize_t hsize[1];

  hsize[0] = array.size;

  hid_t hdf5_standard_type = bsp_get_hdf5_standard_type(array.type);
  hid_t fspace = H5Screate_simple(1, hsize, NULL);
  hid_t lcpl = H5Pcreate(H5P_LINK_CREATE);

  hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);

  // Choose 1 MiB, the default chunk cache size, as our chunk size.
  size_t chunk_size = 1024 * 1024 / bsp_type_size(array.type);

  // If the dataset is smaller than the chunk size, cap the chunk size.
  if (array.size < chunk_size) {
    chunk_size = array.size;
  }

  hsize[0] = chunk_size;
  H5Pset_chunk(dcpl, 1, hsize);

  if (compression_level > 0) {
    H5Pset_deflate(dcpl, compression_level);
  }

  hid_t dset =
      H5Dcreate2(f, label, hdf5_standard_type, fspace, lcpl, dcpl, H5P_DEFAULT);

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
  H5Pclose(dcpl);

  return 0;
}

bsp_array_t bsp_read_array(hid_t f, const char* label) {
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

void bsp_write_attribute(hid_t f, const char* label, const char* string) {
  hid_t strtype = H5Tcopy(H5T_C_S1);
  H5Tset_size(strtype, strlen(string));
  H5Tset_cset(strtype, H5T_CSET_UTF8);
  hid_t dataspace = H5Screate(H5S_SCALAR);

  hid_t attribute =
      H5Acreate2(f, label, strtype, dataspace, H5P_DEFAULT, H5P_DEFAULT);

  H5Awrite(attribute, strtype, string);

  H5Tclose(strtype);
  H5Aclose(attribute);
  H5Sclose(dataspace);
}

char* bsp_read_attribute(hid_t f, const char* label) {
  hid_t attribute = H5Aopen(f, label, H5P_DEFAULT);
  hid_t strtype = H5Aget_type(attribute);

  hid_t type_class = H5Tget_class(strtype);
  assert(type_class == H5T_STRING);

  size_t size = H5Tget_size(strtype);

  char* string = (char*) malloc(size + 1);

  H5Aread(attribute, strtype, string);

  H5Aclose(attribute);
  H5Tclose(strtype);

  return string;
}
