/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USE_HDF5
#include <binsparse/detail/allocator.h>
#include <hdf5.h>

#if __STDC_VERSION__ >= 201112L
bsp_matrix_t bsp_read_matrix_from_group_parallel(hid_t f, int num_threads);
#endif

bsp_matrix_t bsp_read_matrix_from_group(hid_t f);
bsp_matrix_t bsp_read_matrix_from_group_allocator(hid_t f,
                                                  bsp_allocator_t allocator);
#endif

#if __STDC_VERSION__ >= 201112L
bsp_matrix_t bsp_read_matrix_parallel(const char* file_name, const char* group,
                                      int num_threads);
#endif

bsp_matrix_t bsp_read_matrix(const char* file_name, const char* group);
bsp_matrix_t bsp_read_matrix_allocator(const char* file_name, const char* group,
                                       bsp_allocator_t allocator);

#ifdef __cplusplus
}
#endif
