#ifndef ALISP_LS_H
#define ALISP_LS_H

#include <stdbool.h>
#include <stdint.h>
#include "alisp/macros.h"

#define _a_ls_do(ls, i, _next)				\
  for (struct a_ls *i = (ls)->next, *_next = i->next;	\
       i != (ls);					\
       i = _next, _next = i->next)

#define a_ls_do(ls, i)				\
  _a_ls_do(ls, i, a_unique(next))

struct a_ls {
  struct a_ls *prev, *next;
};

void a_ls_init(struct a_ls *self);
bool a_ls_null(const struct a_ls *self);

void a_ls_push(struct a_ls *self, struct a_ls *it);
struct a_ls *a_ls_pop(struct a_ls *self);

uint32_t a_ls_count(struct a_ls *self);

#endif
