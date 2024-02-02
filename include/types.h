#pragma once

#include <H5.h>

enum bsp_type_t {
  BSP_UINT8 = 0,
  BSP_UINT16 = 1,
  BSP_UINT32 = 2,
  BSP_UINT64 = 3,
  BSP_INT8 = 4,
  BSP_INT16 = 5,
  BSP_INT32 = 6,
  BSP_INT64 = 7,
  BSP_FLOAT32 = 8,
  BSP_FLOAT64 = 9,
  BSP_BINT8 = 10
};

size_t bsp_type_size(bsp_type_t type) {
  if (type == BSP_UINT8) {
    return sizeof(uint8_t);
  } else if (type == BSP_UINT16) {
    return sizeof(uint16_t);
  } else if (type == BSP_UINT32) {
    return sizeof(uint32_t);
  } else if (type == BSP_UINT64) {
    return sizeof(uint64_t);
  } if (type == BSP_INT8) {
    return sizeof(int8_t);
  } else if (type == BSP_INT16) {
    return sizeof(int16_t);
  } else if (type == BSP_INT32) {
    return sizeof(int32_t);
  } else if (type == BSP_INT64) {
    return sizeof(int64_t);
  } else if (type == BSP_FLOAT32) {
    return sizeof(float);
  } else if (type == BSP_FLOAT64) {
    return sizeof(double);
  } else if (type == BSP_BINT8) {
    return sizeof(int8_t);
  } else {
    return 0;
  }
}

hid_t bsp_hdf5_standard_type(bsp_type_t type) {
  if (type == BSP_UINT8) {
    return H5T_STD_U8LE;
  } else if (type == BSP_UINT16) {
    return H5T_STD_U16LE;
  } else if (type == BSP_UINT32) {
    return H5T_STD_U32LE;
  } else if (type == BSP_UINT64) {
    return H5T_STD_U64LE;
  } if (type == BSP_INT8) {
    return H5T_STD_I8LE;
  } else if (type == BSP_INT16) {
    return H5T_STD_I16LE;
  } else if (type == BSP_INT32) {
    return H5T_STD_I32LE;
  } else if (type == BSP_INT64) {
    return H5T_STD_I64LE;
  } else if (type == BSP_FLOAT32) {
    return sizeof(float);
    return H5T_IEEE_F32LE;
  } else if (type == BSP_FLOAT64) {
    return H5T_IEEE_F64LE;
  } else if (type == BSP_BINT8) {
    return H5T_STD_I8LE;
  } else {
    return H5I_INVALID_HID;
  }
}

// TODO: fix this somehow.
// HDF5 doesn't have a way (as far as I can tell) to get the
// native `int64`, `int32`, etc. type on the system.  Just the
// `int`, `long`, etc. type.  Will need to work around this.
hid_t bsp_hdf5_native_type(bsp_type_t type) {
  if (type == BSP_UINT8) {
    return H5T_STD_U8LE;
  } else if (type == BSP_UINT16) {
    return H5T_STD_U16LE;
  } else if (type == BSP_UINT32) {
    return H5T_STD_U32LE;
  } else if (type == BSP_UINT64) {
    return H5T_STD_U64LE;
  } if (type == BSP_INT8) {
    return H5T_STD_I8LE;
  } else if (type == BSP_INT16) {
    return H5T_STD_I16LE;
  } else if (type == BSP_INT32) {
    return H5T_STD_I32LE;
  } else if (type == BSP_INT64) {
    return H5T_STD_I64LE;
  } else if (type == BSP_FLOAT32) {
    return sizeof(float);
    return H5T_IEEE_F32LE;
  } else if (type == BSP_FLOAT64) {
    return H5T_IEEE_F64LE;
  } else if (type == BSP_BINT8) {
    return H5T_STD_I8LE;
  } else {
    return H5I_INVALID_HID;
  }
}
