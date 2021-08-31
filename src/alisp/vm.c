#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/prim.h"
#include "alisp/scope.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_vm *a_vm_init(struct a_vm *self) {
  self->next_type_id = 0;
  a_ls_init(&self->code);

  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_push_scope(self, &self->main);

  a_ls_init(&self->frames);
  a_ls_init(&self->stack);
  a_abc_lib_init(&self->abc, self);
  a_lib_import(&self->abc.lib);
  a_math_lib_init(&self->math, self);
  a_lib_import(&self->math.lib);

  memset(self->regs, 0, sizeof(self->regs));
  return self;
}

void a_vm_deinit(struct a_vm *self) {  
  for (int i = 0; i < A_REG_COUNT; i++) {
    struct a_val *v = self->regs[i];

    if (v) {
      a_val_deref(v);
      a_free(self, v);
    }
  }
  
  a_ls_do(&self->stack, vls) {
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_val_deref(v);
    a_free(self, v);
  }

  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, ls);
    a_scope_deref(s);
    a_free(self, s);
  }

  a_ls_do(&self->code, ols) {
    struct a_op *o = a_baseof(ols, struct a_op, pc);
    a_op_deinit(o);
    a_free(self, o);
  }

  a_lib_deinit(&self->math.lib);
  a_lib_deinit(&self->abc.lib);
}

a_pc_t a_pc(struct a_vm *self) { return self->code.prev; }

struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type) {
  struct a_op *op = a_op_init(a_malloc(self, sizeof(struct a_op)), op_type);
  a_ls_push(&self->code, &op->pc);
  return op;
}

void a_analyze(struct a_vm *self, a_pc_t pc) {
  struct a_ls stack = self->stack;
  a_ls_init(&self->stack);
  
  while (pc && pc != &self->code) {
    struct a_op *op = a_baseof(pc, struct a_op, pc);
    pc = a_op_analyze(op, self);
  }
  
  a_reset(self);
  self->stack = stack;
}


struct a_scope *a_scope(struct a_vm *self) {
  return a_baseof(self->scopes.prev, struct a_scope, ls);
}

struct a_scope *a_push_scope(struct a_vm *self, struct a_scope *scope) {
  a_ls_push(&self->scopes, &scope->ls);
  return scope;
}

struct a_scope *a_begin(struct a_vm *self) {
  struct a_scope *s = a_malloc(self, sizeof(struct a_scope));
  a_scope_init(s, self, a_scope(self));
  return a_push_scope(self, s);
}

struct a_scope *a_end(struct a_vm *self) {
  struct a_ls *ls = self->scopes.prev;
  assert(ls != &self->scopes);
  a_ls_pop(ls);
  return a_baseof(ls, struct a_scope, ls);
}

void a_store(struct a_vm *self, a_reg_t reg, struct a_val *val) {
  struct a_val *prev = self->regs[reg];

  if (prev) {
    a_val_deref(prev);
    a_free(self, prev);
  }
  
  self->regs[reg] = val;

}

void *a_malloc(struct a_vm *vm, uint32_t size) { return malloc(size); }

void a_free(struct a_vm *vm, void *p) { free(p); }
