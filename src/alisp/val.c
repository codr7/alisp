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
  assert(source->type->copy_val);
  source->type->copy_val(self, source);
}

void a_dump(struct a_val *self) {
  assert(self->type->dump_val);
  self->type->dump_val(self);
}

bool a_true(struct a_val *self) {
  assert(self->type->true_val);
  return self->type->true_val(self);
}

