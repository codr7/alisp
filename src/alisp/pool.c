#include "alisp/pool.h"
#include "alisp/vm.h"

struct a_pool *a_pool_init(struct a_pool *self, struct a_vm *vm, uint32_t page_size, uint32_t slot_size) {
  self->vm = vm;
  self->page_size = slot_size + page_size*slot_size;
  self->slot_size = slot_size;
  a_ls_init(&self->pages);
  return self;
}

void a_pool_deinit(struct a_pool *self) {
  a_ls_do(&self->pages, ls) { a_free(self->vm, a_baseof(a_ls_pop(ls), struct a_page, ls)); }
}

void *a_pool_alloc(struct a_pool *self) {
  a_ls_do(&self->pages, ls) {
    struct a_page *p = a_baseof(ls, struct a_page, ls);

    if (p->offset + self->slot_size <= self->page_size) {
      p->offset += self->slot_size;
      return (uint8_t *)p->slots + p->offset;
    }

    break;
  }

  struct a_page *p = a_malloc(self->vm, sizeof(struct a_page) + self->page_size);
  p->offset = a_align(p->slots, self->slot_size) - p->slots;
  void *s = (uint8_t *)p->slots + p->offset;
  p->offset += self->slot_size;
  a_ls_push(self->pages.next, &p->ls);
  return s;
}
