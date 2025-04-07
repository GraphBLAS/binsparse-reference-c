#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char* key_with_index(const char* key, size_t index);

#ifdef BSP_USE_HDF5
#include <hdf5.h>

bsp_tensor_t bsp_read_tensor_from_group(hid_t f);
#endif

bsp_tensor_t bsp_read_tensor(const char* file_name, const char* group);

#ifdef __cplusplus
}
#endif
