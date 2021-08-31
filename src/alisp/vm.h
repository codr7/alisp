#ifndef ALISP_VM_H
#define ALISP_VM_H

#include "alisp/libs/abc.h"
#include "alisp/libs/math.h"
#include "alisp/ls.h"
#include "alisp/op.h"
#include "alisp/pool.h"
#include "alisp/scope.h"
#include "alisp/types.h"
#include "alisp/val.h"

#define A_VERSION 5

#define A_PAGE_SIZE 32000
#define A_DEFAULT_PAGE_SIZE 32

#define A_BINDING_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_FORM_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_FRAME_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_FUNC_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_LS_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_OP_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_PRIM_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_SCOPE_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_STRING_PAGE_SIZE A_DEFAULT_PAGE_SIZE
#define A_VAL_PAGE_SIZE A_DEFAULT_PAGE_SIZE

struct a_vm {
  struct a_pool pool,
    binding_pool, ls_pool,
    form_pool, frame_pool, func_pool,
    op_pool, prim_pool,
    scope_pool, string_pool,
    val_pool;

  a_type_id next_type_id;
  struct a_abc_lib abc;
  struct a_math_lib math;
  
  struct a_ls code, frames, scopes, stack;  
  struct a_scope main;
  struct a_val *regs[A_REG_COUNT];
};

struct a_vm *a_vm_init(struct a_vm *self);
void a_vm_deinit(struct a_vm *self);

a_pc_t a_pc(struct a_vm *self);
struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type);
void a_analyze(struct a_vm *self, a_pc_t pc);
bool a_eval(struct a_vm *self, a_pc_t pc);

struct a_scope *a_scope(struct a_vm *self);

struct a_scope *a_push_scope(struct a_vm *self, struct a_scope *scope);
struct a_scope *a_begin(struct a_vm *self);
struct a_scope *a_end(struct a_vm *self);

void a_store(struct a_vm *self, a_reg_t reg, struct a_val *val);

#endif
