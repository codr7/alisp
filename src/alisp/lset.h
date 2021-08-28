#ifndef ALISP_LSET_H
#define ALISP_LSET_H

#include <stdint.h>
#include "alisp/compare.h"
#include "alisp/ls.h"

typedef const void *(*a_lset_key)(const struct a_ls *x);
typedef uint16_t a_lset_count;

struct a_lset {
  a_lset_key key;
  a_compare_t compare;
  struct a_ls items;
};

struct a_lset *a_lset_init(struct a_lset *self, a_lset_key key, a_compare_t compare);
struct a_ls *a_lset_insert(struct a_lset *self, struct a_ls *item, bool force);
struct a_ls *a_lset_find(struct a_lset *self, const void *key);

#endif
