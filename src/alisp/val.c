#include <assert.h>
#include "alisp/pool.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

struct a_val *a_val(struct a_vm *vm, struct a_type *type) {
  return a_val_init(a_malloc(&vm->val_pool, sizeof(struct a_val)), type);
}

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

enum a_order a_compare(struct a_val *self, struct a_val *other) {
  assert(self->type->compare_val);
  return self->type->compare_val(self, other);
}

void a_copy(struct a_val *self, struct a_val *source) {
  assert(source->type->copy_val);
  source->type->copy_val(self, source);
}

void a_dump(struct a_val *self) {
  assert(self->type->dump_val);
  self->type->dump_val(self);
}

bool a_equals(struct a_val *self, struct a_val *other) {
  assert(self->type->equals_val);
  return self->type->equals_val(self, other);
}

bool a_is(struct a_val *self, struct a_val *other) {
  assert(self->type->is_val);
  return self->type->is_val(self, other);
}

bool a_true(struct a_val *self) {
  assert(self->type->true_val);
  return self->type->true_val(self);
}

