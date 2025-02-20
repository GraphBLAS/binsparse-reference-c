/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// TODO: make cJSON optional.

#include<binsparse/tensor.h>
#include <cJSON/cJSON.h>

#ifdef BSP_USE_HDF5
#include <hdf5.h>

int bsp_write_tensor_to_group(hid_t f, bsp_tensor_t tensor, cJSON* user_json,
                              int compression_level);
#endif

int bsp_write_tensor(const char* fname, bsp_tensor_t tensor, const char* group,
                     cJSON* user_json, int compression_level);

#ifdef __cplusplus
}
#endif
