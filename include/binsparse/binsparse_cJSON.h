/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/detail/allocator.h>
#include <binsparse/matrix.h>

#include <cJSON/cJSON.h>

#ifndef BSP_BINSPARSE_CJSON_H
#define BSP_BINSPARSE_CJSON_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

bsp_error_t bsp_write_matrix_cjson(const char* fname, bsp_matrix_t matrix,
                                   const char* group, cJSON* user_json,
                                   int compression_level);

#ifdef BSP_BINSPARSE_HDF5_H
bsp_error_t bsp_write_matrix_to_group_cjson(hid_t f, bsp_matrix_t matrix,
                                            cJSON* user_json,
                                            int compression_level);
#endif

#ifdef __cplusplus
}
#endif
