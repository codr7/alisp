#include <assert.h>
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/multi.h"
#include "alisp/string.h"
#include "alisp/vm.h"

static const void *funcs_key(const struct a_ls *self) {
  return &a_baseof(self, struct a_func, ls)->weight;
}

static enum a_order funcs_compare(const void *x, const void *y) {
  const a_func_weight_t *xw = x, *yw = y;
  if (*xw > *yw) { return A_LT; }
  return (*xw == *yw) ? A_EQ : A_GT;
}

struct a_multi *a_multi(struct a_vm *vm, struct a_string *name, uint8_t arg_count) {
  return a_multi_init(a_pool_alloc(&vm->multi_pool), name, arg_count);
}

struct a_multi *a_multi_init(struct a_multi *self, struct a_string *name, uint8_t arg_count) {
  self->name = name;
  self->arg_count = arg_count;
  a_lset_init(&self->funcs, funcs_key, funcs_compare);
  return self;
}

bool a_multi_add(struct a_multi *self, struct a_func *func) {
  if (func->args.count != self->arg_count) {
    a_fail("Func has wrong number of arguments");
    return false;
  }

  return a_lset_add(&self->funcs, &func->ls, false);
}

struct a_func *a_multi_specialize(struct a_multi *self, struct a_vm *vm, bool full) {
  a_type_id_t w = 0;
    
  a_ls_do(&self->funcs.items, ls) {
    struct a_func *f = a_baseof(ls, struct a_func, ls);
    if (!full && f->weight < w) { break; }
    if (a_func_applicable(f, vm)) { return f; }
    w = f->weight;
  }

  return NULL;
}

a_pc_t a_multi_call(struct a_multi *self, struct a_vm *vm, enum a_call_flags flags, a_pc_t ret) {
  struct a_func *f = a_multi_specialize(self, vm, true);

  if (!f) {
    a_fail("Not applicable: %s", self->name->data);
    return NULL;
  }

  return a_func_call(f, vm, flags, ret);
}
