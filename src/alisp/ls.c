#include <stddef.h>
#include "alisp/ls.h"

void a_ls_init(struct a_ls *self) { self->prev = self->next = self; }

bool a_ls_null(struct a_ls *self) { return self->prev == self->next; }

void a_ls_insert(struct a_ls *self, struct a_ls *it) {
  it->prev = self->prev;
  self->prev->next = it;
  it->next = self;
  self->prev = it;
}

struct a_ls *a_ls_remove(struct a_ls *self) {
  self->prev->next = self->next;
  self->next->prev = self->prev;
  return self;
}

void a_ls_prepend(struct a_ls *self, struct a_ls *it) {
  a_ls_insert(self->next, it);
}

void a_ls_append(struct a_ls *self, struct a_ls *it) {
  a_ls_insert(self, it);
}

struct a_ls *a_ls_pop_first(struct a_ls *self) {
  struct a_ls *i = self->next;
  return (i == self) ? NULL : a_ls_remove(i);
}

struct a_ls *a_ls_pop_last(struct a_ls *self) {
  struct a_ls *i = self->prev;
  return (i == self) ? NULL : a_ls_remove(i);
}
