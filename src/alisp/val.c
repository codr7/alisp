#include <assert.h>
#include <stdio.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

struct a_val *a_val_new(struct a_type *type) {
  a_ls_do(&type->vm->free_vals, ls) {
    return a_val_init(a_baseof(a_ls_pop(ls), struct a_val, ls), type);
  }
  
  return a_val_init(a_pool_alloc(&type->vm->val_pool), type);
}

struct a_val *a_val_init(struct a_val *self, struct a_type *type) {
  self->type = type;
  self->undef = false;
  return self;
}

void a_val_free(struct a_val *self, struct a_vm *vm) {
  a_deref(self);
  a_ls_push(vm->free_vals.next, &self->ls);
}

a_pc_t a_call(struct a_val *self, enum a_call_flags flags, a_pc_t ret) {
  assert(!self->undef);
  assert(self->type->call_val);
  return self->type->call_val(self, flags, ret);
}

enum a_order a_compare(struct a_val *self, struct a_val *other) {
  assert(self->type == other->type);
  assert(!self->undef);
  assert(self->type->compare_val);
  return self->type->compare_val(self, other);
}

struct a_val *a_copy(struct a_val *self, struct a_val *source) {
  if (!(self->undef = source->undef)) {
    assert(source->type->copy_val);
    source->type->copy_val(self, source);
  }
  
  return self;
}

bool a_deref(struct a_val *self) {
  return (self->type->deref_val && !self->undef) ? self->type->deref_val(self) : true;
}

void a_dump(struct a_val *self) {
  assert(!self->undef);
  assert(self->type->dump_val);
  self->type->dump_val(self);
}

bool a_equals(struct a_val *self, struct a_val *other) {
  if (self->type != other->type) { return false; }
  assert(!self->undef);
  assert(self->type->equals_val);
  return self->type->equals_val(self, other);
}

bool a_is(struct a_val *self, struct a_val *other) {
  assert(!self->undef);
  assert(self->type->is_val);
  return self->type->is_val(self, other);
}

struct a_iter *a_iter(struct a_val *self) {
  assert(!self->undef);
  assert(self->type->iter_val);
  return self->type->iter_val(self);
}

bool a_true(struct a_val *self) {
  assert(!self->undef);
  assert(self->type->true_val);
  return self->type->true_val(self);
}

