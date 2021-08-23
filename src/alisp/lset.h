#ifndef ALISP_LSET_H
#define ALISP_LSET_H

#include "alisp/compare.h"
#include "alisp/ls.h"

typedef const void *(*a_lset_key)(const struct a_ls *x);

struct a_lset {
  a_lset_key key;
  a_compare compare;
  struct a_ls items;
};

struct a_lset *a_lset_init(struct a_lset *self, a_lset_key key, a_compare compare);
struct a_ls *a_lset_insert(struct a_lset *self, struct a_ls *item);
struct a_ls *a_lset_find(struct a_lset *self, const void *key);

#endif
