#include <stddef.h>
#include "alisp/ls.h"

void a_ls_init(struct a_ls *self) { self->prev = self->next = self; }

bool a_ls_null(const struct a_ls *self) { return self->prev == self && self->next == self; }

void a_ls_push(struct a_ls *self, struct a_ls *it) {
  it->prev = self->prev;
  self->prev->next = it;
  it->next = self;
  self->prev = it;
}

struct a_ls *a_ls_pop(struct a_ls *self) {
  self->prev->next = self->next;
  self->next->prev = self->prev;
  return self;
}

uint32_t a_ls_count(struct a_ls *self) {
  uint32_t n = 0;
  for (struct a_ls *i = self->next; i != self; i = i->next, n++);
  return n;
}

