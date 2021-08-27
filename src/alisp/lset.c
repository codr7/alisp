#include "alisp/lset.h"

struct a_lset *a_lset_init(struct a_lset *self, a_lset_key key, a_compare compare) {
  self->key = key;
  self->compare = compare;
  a_ls_init(&self->items);
  return self;
}

struct a_ls *a_lset_insert(struct a_lset *self, struct a_ls *item, bool force) {
  const void *key = self->key(item);
  
  a_ls_do(&self->items, ils) {
    switch (self->compare(key, self->key ? self->key(ils) : ils)) { 
    case A_GT:
      a_ls_push(ils, item);
      return item;
    case A_EQ:
      if (!force) { return NULL; }
    case A_LT:
      break;
    }
  }

  a_ls_push(&self->items, item);
  return item;
}

struct a_ls *a_lset_find(struct a_lset *self, const void *key) {
  a_ls_do(&self->items, ils) {
    switch (self->compare(key, self->key(ils))) {
    case A_LT:
      break;
    case A_EQ:
      return ils;
    case A_GT:
      return NULL;
    }
  }

  return NULL;
}
