/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <binsparse/detail/allocator.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
  int id;
  size_t size;
} bsp_shm_t;

static inline bsp_shm_t bsp_shm_new(size_t size) {
  bsp_shm_t shm;
  shm.size = size;

  if ((shm.id = shmget(IPC_PRIVATE, size,
                       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) < 0) {
    perror("shmget");
  }

  return shm;
}

static inline void bsp_shm_delete(bsp_shm_t shm) {
  shmctl(shm.id, IPC_RMID, 0);
}

static inline void* bsp_shm_attach(bsp_shm_t shm) {
  void* data;

  if ((data = shmat(shm.id, NULL, 0)) == (void*) -1) {
    perror("write");
  }

  return data;
}

static inline void bsp_shm_detach(void* data) {
  shmdt(data);
}

static inline void* bsp_shm_malloc(size_t size) {
  bsp_shm_t shm_id = bsp_shm_new(size);

  void* ptr = bsp_shm_attach(shm_id);

  bsp_shm_delete(shm_id);

  return ptr;
}

static inline void bsp_shm_free(void* ptr) {
  bsp_shm_detach(ptr);
}

const static bsp_allocator_t bsp_shm_allocator = {.malloc = bsp_shm_malloc,
                                                  .free = bsp_shm_free};
