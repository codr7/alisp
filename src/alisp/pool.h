#ifndef ALISP_POOL_H
#define ALISP_POOL_H

#include <stdint.h>
#include "alisp/ls.h"
#include "alisp/types.h"

struct a_slot {
  struct a_ls pool_free;
  uint32_t size;
  uint8_t data[];
};
  
struct a_page {
  struct a_ls pool_pages;
  uint32_t offset;
  struct a_slot slots[];
};
  
struct a_pool {
  struct a_pool *source;
  uint32_t page_size;
  struct a_ls pages, free;
  a_refs refs;
};


struct a_pool *a_pool_init(struct a_pool *self, struct a_pool *source, uint32_t page_size, uint32_t slot_size);
struct a_pool *a_pool_ref(struct a_pool *self);
bool a_pool_deref(struct a_pool *self);
void *a_malloc(struct a_pool *self, uint32_t size);
void a_free(struct a_pool *self, void *data);

#endif
