#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/vm.h"

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super_types[]) {
  self->vm = vm;
  assert(vm->next_type_id < A_MAX_TYPE_ID);
  self->id = vm->next_type_id++;
  self->name = a_string_ref(name);
  memset(self->super_types, 0, sizeof(self->super_types));
  self->super_types[self->id] = self;
  
  for (struct a_type **st = super_types; *st; st++) {
    for (a_type_id id = 0; id < A_MAX_TYPE_ID; id++) {
      struct a_type *t = (*st)->super_types[id];
      if (t) { self->super_types[id] = t; }
    }
    
    self->super_types[(*st)->id] = *st;
  }
  
  self->call_val = NULL;
  self->copy_val = NULL;
  self->deref_val = NULL;

  self->ref_count = 1;
  return self;
}

struct a_type *a_type_ref(struct a_type *self) {
  self->ref_count++;
  return self;
}

bool a_type_deref(struct a_type *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  a_string_deref(self->name);
  return true;
}

bool a_isa(struct a_type *self, struct a_type *super) {
  return self->super_types[super->id];
}
