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

#include <binsparse/matrix.h>

bsp_error_t bsp_write_matrix(const char* fname, bsp_matrix_t matrix,
                             const char* group, const char* user_json,
                             int compression_level);

#ifdef __cplusplus
}
#endif
