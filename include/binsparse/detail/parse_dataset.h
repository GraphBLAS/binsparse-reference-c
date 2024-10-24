/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <string.h>

typedef struct {
  char* fname;
  char* dataset;
} bsp_fdataset_info_t;

static inline bsp_fdataset_info_t bsp_parse_fdataset_string(char* str) {
  size_t len = strlen(str);

  int split = -1;
  for (int i = len - 1; i >= 0; i--) {
    if (str[i] == ':') {
      split = i;
      break;
    }
  }

  if (split == -1) {
    bsp_fdataset_info_t info;
    info.fname = (char*) malloc(sizeof(char) * (len + 1));
    strcpy(info.fname, str);
    info.dataset = NULL;
    return info;
  } else {
    bsp_fdataset_info_t info;
    info.fname = (char*) malloc(sizeof(char) * (split + 1));
    strncpy(info.fname, str, split);
    info.fname[split] = '\0';
    info.dataset = (char*) malloc(sizeof(char) * (len - split));
    strcpy(info.dataset, &str[split + 1]);
    return info;
  }
}

static inline const char* bsp_get_file_extension(const char* file_name) {
  int64_t len = strlen(file_name);
  for (int64_t i = len - 1; i >= 0; i--) {
    if (file_name[i] == '.') {
      return &file_name[i];
    }
  }
  return NULL;
}
