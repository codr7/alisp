#include "alisp/binding.h"
#include "alisp/string.h"

void a_binding_deinit(struct a_binding *self) {
    a_val_deref(&self->val);
}

const void *a_binding_key(const struct a_ls *self) {
  return a_baseof(a_baseof(self, struct a_val, ls), struct a_binding, val)->key;
}

enum a_order a_binding_compare(const void *x, const void *y) {
  return a_string_compare(x, y);
}
