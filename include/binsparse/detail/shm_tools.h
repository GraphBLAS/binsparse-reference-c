#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
  int id;
  size_t size;
} bsp_shm_t;

bsp_shm_t bsp_shm_new(size_t size) {
  bsp_shm_t shm;
  shm.size = size;

  if ((shm.id = shmget(IPC_PRIVATE, size,
                       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) < 0) {
    perror("shmget");
  }

  return shm;
}

void bsp_shm_delete(bsp_shm_t shm) {
  shmctl(shm.id, IPC_RMID, 0);
}

void* bsp_shm_attach(bsp_shm_t shm) {
  void* data;

  if ((data = shmat(shm.id, NULL, 0)) == (void*) -1) {
    perror("write");
  }

  return data;
}

void bsp_shm_detach(void* data) {
  shmdt(data);
}