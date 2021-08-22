#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include "alisp/fail.h"
#include "alisp/pool.h"

struct a_pool *a_pool_init(struct a_pool *self, struct a_pool *source, uint32_t page_size, uint32_t slot_size) {
  self->source = source ? a_pool_ref(source) : NULL;
  self->page_size = a_align(0, slot_size) + page_size*(a_align(0, slot_size) + sizeof(struct a_slot) + slot_size);
  a_ls_init(&self->pages);
  a_ls_init(&self->free);
  self->refs = 1;
  return self;
}

struct a_pool *a_pool_ref(struct a_pool *self) {
  self->refs++;
  return self;
}

bool a_pool_deref(struct a_pool *self) {
  assert(self->refs);
  if (--self->refs) { return false; }

  a_ls_do(&self->pages, pls) {
    struct a_page *p = a_baseof(pls, struct a_page, pool_items);
    if (self->source) {
      a_pool_free(self->source, p);
    } else {
      free(p);
    }
  }

  if (self->source) { a_pool_deref(self->source); }
  return true;
}

void *a_pool_malloc(struct a_pool *self, uint32_t size) {
  if (size > self->page_size) { a_fail("Malloc exceeds page size: %" PRIu32, size); }

  a_ls_do(&self->free, sls) {
    struct a_slot *s = a_baseof(sls, struct a_slot, pool_free);

    if (s->size == size) {
      a_ls_remove(&s->pool_free);
      return s->data;
    }
  }
  
  a_ls_do(&self->pages, pls) {
    struct a_page *p = a_baseof(pls, struct a_page, pool_items);
    
    if (p->offset+size <= self->page_size) {
      uint32_t ss = sizeof(struct a_slot) + size;
      struct a_slot *s = (struct a_slot *)a_align((uint8_t *)p->slots + p->offset, ss);
      s->size = size;
      p->offset = (uint8_t *)s - (uint8_t *)p->slots + ss;
      return s->data;
    }
  }

  uint32_t ps = sizeof(struct a_page) + self->page_size;
  struct a_page *p = self->source ? a_pool_malloc(self->source, ps) : malloc(ps);
  p->offset = size;
  a_ls_prepend(&self->pages, &p->pool_items);
  return p->slots;
}

void a_pool_free(struct a_pool *self, void *slot) {
  struct a_slot *s = a_baseof(slot, struct a_slot, data);
  a_ls_prepend(&self->free, &s->pool_free);
}
