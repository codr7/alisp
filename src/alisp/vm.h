#ifndef ALISP_VM_H
#define ALISP_VM_H

#include "alisp/libs/abc.h"
#include "alisp/ls.h"
#include "alisp/op.h"
#include "alisp/pool.h"
#include "alisp/scope.h"
#include "alisp/types.h"
#include "alisp/val.h"

#define A_PAGE_SIZE 32000
#define A_DEFAULT_PAGE_SIZE 32

#define A_BINDING_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_OP_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_SCOPE_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_STRING_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_VAL_PAGE_SIZE A_DEFAULT_PAGE_SIZE

#define A_REG_COUNT 64

struct a_vm {
  struct a_pool pool;
  struct a_pool binding_pool, op_pool, scope_pool, string_pool, val_pool;

  a_type_id next_type_id;
  struct a_abc_lib abc;
  
  struct a_ls code;  
  struct a_scope main;
  struct a_ls scopes;
  struct a_val regs[A_REG_COUNT];
  struct a_ls stack;
};

struct a_vm *a_vm_init(struct a_vm *self);
void a_vm_deinit(struct a_vm *self);

a_pc a_next_pc(struct a_vm *self);
struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type);
void a_eval(struct a_vm *self, a_pc pc);

struct a_scope *a_scope(struct a_vm *self);

struct a_val *a_push(struct a_vm *self, struct a_type *type);
struct a_val *a_peek(struct a_vm *self);
struct a_val *a_pop(struct a_vm *self);

a_reg a_bind_reg(struct a_vm *self, struct a_string *key);

#endif
