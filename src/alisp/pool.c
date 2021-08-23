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
  self->ref_count = 1;
  return self;
}

struct a_pool *a_pool_ref(struct a_pool *self) {
  self->ref_count++;
  return self;
}

bool a_pool_deref(struct a_pool *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }


  a_ls_do(&self->pages, pls) {
    struct a_page *p = a_baseof(pls, struct a_page, pool_pages);
    a_ls_pop(&p->pool_pages);
    
    if (self->source) {
      a_free(self->source, p);
    } else {
      free(p);
    }
  }

  if (self->source) { a_pool_deref(self->source); }
  return true;
}

void *a_malloc(struct a_pool *self, uint32_t size) {
  a_ls_do(&self->free, sls) {
    struct a_slot *s = a_baseof(sls, struct a_slot, pool_free);

    if (s->size == size) {
      a_ls_pop(&s->pool_free);
      return s->data;
    }
  }

  uint32_t ss = sizeof(struct a_slot) + size;

  a_ls_do(&self->pages, pls) {
    struct a_page *p = a_baseof(pls, struct a_page, pool_pages);    
    struct a_slot *s = (struct a_slot *)a_align((uint8_t *)p->slots + p->offset, ss);
    uint32_t new_offset = (uint8_t *)s - (uint8_t *)p->slots + ss;
    
    if (new_offset <= self->page_size) {
      s->size = size;
      p->offset = new_offset;
      return s->data;
    }
  }

  uint32_t ps = sizeof(struct a_page) + self->page_size;
  struct a_page *p = self->source ? a_malloc(self->source, ps) : malloc(ps);
  struct a_slot *s = a_align(p->slots, ss);
  uint32_t new_offset = (uint8_t *)s - (uint8_t *)p->slots + ss;
  if (new_offset > self->page_size) { a_fail("Malloc exceeds page size: %" PRIu32, size); }
  s->size = size;
  p->offset = new_offset;
  a_ls_push(&self->pages, &p->pool_pages);
  return s->data;
}

void a_free(struct a_pool *self, void *data) {
  struct a_slot *s = a_baseof(data, struct a_slot, data);
  a_ls_push(self->free.next, &s->pool_free);
}
