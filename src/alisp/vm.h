#ifndef ALISP_VM_H
#define ALISP_VM_H

#include "alisp/frame.h"
#include "alisp/libs/abc.h"
#include "alisp/libs/math.h"
#include "alisp/limits.h"
#include "alisp/ls.h"
#include "alisp/op.h"
#include "alisp/scope.h"
#include "alisp/val.h"

#define A_VERSION 6

struct a_vm {
  a_type_id next_type_id;
  struct a_abc_lib abc;
  struct a_math_lib math;
  
  struct a_ls code, scopes, stack;  
  struct a_scope main;

  struct a_frame frames[A_FRAME_COUNT];
  uint16_t frame_count;
  
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

struct a_frame *a_push_frame(struct a_vm *self, struct a_func *func, enum a_call_flags flags, a_pc_t ret);
struct a_frame *a_pop_frame(struct a_vm *self);

void a_store(struct a_vm *self, a_reg_t reg, struct a_val *val);

void *a_malloc(struct a_vm *vm, uint32_t size);
void a_free(struct a_vm *vm, void *p);


#endif
