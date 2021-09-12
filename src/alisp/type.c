#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/vm.h"

static void clone_val(struct a_val *dst, struct a_val *src) { a_copy(dst, src); }

static bool equals_val(struct a_val *x, struct a_val *y) { return a_is(x, y); }

static bool true_val(struct a_val *val) { return true; }

struct a_type *a_type_new(struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  return a_type_init(a_pool_alloc(&vm->type_pool), vm, name, super);
}

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  self->vm = vm;
  assert(vm->next_type_id < A_MAX_TYPE_ID);
  self->id = vm->next_type_id++;
  self->name = name;
  memset(self->super, 0, sizeof(self->super));
  self->super[self->id] = self;
  
  for (struct a_type **st = super; *st; st++) {
    for (a_type_id_t id = 0; id <= (*st)->id; id++) {
      struct a_type *t = (*st)->super[id];
      if (t && !self->super[t->id]) { self->super[t->id] = t; }
    }
  }
  
  self->call_val = NULL;
  self->clone_val = clone_val;
  self->compare_val = NULL;
  self->copy_val = NULL;
  self->deref_val = NULL;
  self->dump_val = NULL;
  self->equals_val = equals_val;
  self->is_val = NULL;
  self->iter_val = NULL;
  self->true_val = true_val;
  return self;
}

struct a_type *a_type_clone(struct a_type *self, struct a_vm *dst_vm) {
  struct a_string *dn = a_string(dst_vm, self->name->data);
  struct a_val *found = a_scope_find(&dst_vm->main, dn);

  if (found) {
    assert(found->type == &dst_vm->abc.meta_type);
    return found->as_meta;
  }

  struct a_type *dst = a_type_new(dst_vm, dn, A_SUPER(NULL));
  dst->call_val = self->call_val;
  dst->clone_val = self->clone_val;
  dst->compare_val = self->compare_val;
  dst->copy_val = self->copy_val;
  dst->deref_val = self->deref_val;
  dst->dump_val = self->dump_val;
  dst->equals_val = self->equals_val;
  dst->is_val = self->is_val;
  dst->iter_val = self->iter_val;
  dst->true_val = self->true_val;

  for (a_type_id_t id = 0; id < self->id; id++) {
    struct a_type *st = self->super[id];

    if (st) {
      struct a_type *dt = a_type_clone(st, dst_vm);
      dst->super[dt->id] = dt;
    }
  }

  a_scope_bind(&dst_vm->main, dn, &dst_vm->abc.meta_type)->as_meta = dst;
  return dst;
}

bool a_isa(struct a_type *self, struct a_type *super) { return self->super[super->id]; }
