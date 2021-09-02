#ifndef ALISP_POOL_H
#define ALISP_POOL_H

#include <stdint.h>
#include "alisp/ls.h"

struct a_vm;

struct a_pool {
  struct a_vm *vm;
  uint32_t page_size, slot_size;
  struct a_ls pages;
};

struct a_page {
  struct a_ls ls;
  uint32_t offset;
  uint8_t slots[];
};

struct a_pool *a_pool_init(struct a_pool *self, struct a_vm *vm, uint32_t page_size, uint32_t slot_size);
void a_pool_deinit(struct a_pool *self);
void *a_pool_alloc(struct a_pool *self);


#endif
