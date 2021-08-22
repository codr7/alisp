#include "alisp/lset.h"

struct a_lset *a_lset_init(struct a_lset *self, a_lset_compare compare) {
  self->compare = compare;
  a_ls_init(&self->items);
  return self;
}
