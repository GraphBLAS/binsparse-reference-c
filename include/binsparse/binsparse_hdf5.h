/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/detail/allocator.h>
#include <binsparse/matrix.h>

#include <hdf5.h>

#ifndef BSP_BINSPARSE_HDF5_H
#define BSP_BINSPARSE_HDF5_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ >= 201112L
bsp_error_t bsp_read_matrix_from_group_parallel(bsp_matrix_t* matrix, hid_t f,
                                                int num_threads);
#endif

bsp_error_t bsp_read_matrix_from_group(bsp_matrix_t* matrix, hid_t f);
bsp_error_t bsp_read_matrix_from_group_allocator(bsp_matrix_t* matrix, hid_t f,
                                                 bsp_allocator_t allocator);

bsp_error_t bsp_write_matrix_to_group(hid_t f, bsp_matrix_t matrix,
                                      const char* user_json,
                                      int compression_level);

#ifdef BSP_BINSPARSE_CJSON_H
bsp_error_t bsp_write_matrix_to_group_cjson(hid_t f, bsp_matrix_t matrix,
                                            cJSON* user_json,
                                            int compression_level);
#endif

#ifdef __cplusplus
}
#endif
