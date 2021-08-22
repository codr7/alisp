#ifndef ALISP_LSET_H
#define ALISP_LSET_H

#include "alisp/compare.h"
#include "alisp/ls.h"

typedef enum a_order (*a_lset_compare)(const struct a_ls *x, const struct a_ls *y);

struct a_lset {
  a_lset_compare compare;
  struct a_ls items;
};

struct a_lset *a_lset_init(struct a_lset *self, a_lset_compare compare);

#endif
