#include "alisp/type.h"
#include "alisp/val.h"

struct a_val *a_val_init(struct a_val *self, struct a_type *type) {
  a_ls_init(&self->vm_stack);
  self->type = type;
  return self;
}

void a_val_deinit(struct a_val *self) {
  if (self->type->deinit_val) { self->type->deinit_val(self); }
}
