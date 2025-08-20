/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/detail/allocator.h>
#include <binsparse/matrix.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ >= 201112L
bsp_error_t bsp_read_matrix_parallel(bsp_matrix_t* matrix,
                                     const char* file_name, const char* group,
                                     int num_threads);
#endif

bsp_error_t bsp_read_matrix(bsp_matrix_t* matrix, const char* file_name,
                            const char* group);
bsp_error_t bsp_read_matrix_allocator(bsp_matrix_t* matrix,
                                      const char* file_name, const char* group,
                                      bsp_allocator_t allocator);

#ifdef __cplusplus
}
#endif
