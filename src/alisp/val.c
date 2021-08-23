#include <assert.h>
#include "alisp/type.h"
#include "alisp/val.h"

struct a_val *a_val_init(struct a_val *self, struct a_type *type) {
  self->type = type;
  return self;
}

bool a_val_deref(struct a_val *self) {
  return self->type->deref_val ? self->type->deref_val(self) : true;
}

a_pc a_call(struct a_val *self, a_pc ret, bool check) {
  assert(self->type->call_val);
  return self->type->call_val(self, ret, check);
}

void a_copy(struct a_val *self, struct a_val *source) {
  assert(self->type->copy_val);
  self->type->copy_val(self, source);
}
