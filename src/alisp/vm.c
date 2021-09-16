#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/frame.h"
#include "alisp/form.h"
#include "alisp/func.h"
#include "alisp/iter.h"
#include "alisp/multi.h"
#include "alisp/parser.h"
#include "alisp/prim.h"
#include "alisp/queue.h"
#include "alisp/scope.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/thread.h"
#include "alisp/utils.h"
#include "alisp/vm.h"

static const void *strings_key(const struct a_ls *ls) { return a_baseof(ls, struct a_string, ls)->data; }
static enum a_order strings_compare(const void *x, const void *y) { return a_strcmp(x, y); }

struct a_vm *a_vm_init(struct a_vm *self) {
  self->next_type_id = 0;
  a_pool_init(&self->form_pool, self, A_PAGE_SIZE, sizeof(struct a_form));
  a_pool_init(&self->func_pool, self, A_PAGE_SIZE, sizeof(struct a_func));
  a_pool_init(&self->iter_pool, self, A_PAGE_SIZE, sizeof(struct a_iter));
  a_pool_init(&self->multi_pool, self, A_PAGE_SIZE, sizeof(struct a_multi));
  a_pool_init(&self->op_pool, self, A_PAGE_SIZE, sizeof(struct a_op));
  a_pool_init(&self->prim_pool, self, A_PAGE_SIZE, sizeof(struct a_prim));
  a_pool_init(&self->queue_pool, self, A_PAGE_SIZE, sizeof(struct a_queue));
  a_pool_init(&self->thread_pool, self, A_PAGE_SIZE, sizeof(struct a_thread));
  a_pool_init(&self->type_pool, self, A_PAGE_SIZE, sizeof(struct a_type));
  a_pool_init(&self->val_pool, self, A_PAGE_SIZE, sizeof(struct a_val));
  a_ls_init(&self->code);
  a_ls_init(&self->free_forms);
  a_ls_init(&self->free_iters);
  a_ls_init(&self->free_queues);  
  a_ls_init(&self->free_threads);
  a_ls_init(&self->free_vals);
  a_lset_init(&self->strings, strings_key, strings_compare);

  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_push_scope(self, &self->main);
  
  self->frame_count = 0;
  a_ls_init(&self->stack);
  memset(self->regs, 0, sizeof(self->regs));
  self->break_depth = 0;
  
  a_abc_lib_init(&self->abc, self);
  a_lib_import(&self->abc.lib);
  a_math_lib_init(&self->math, self);
  a_lib_import(&self->math.lib);
  return self;
}

void a_vm_deinit(struct a_vm *self) {  
  a_ls_do(&self->strings.items, ls) {
    struct a_string *s = a_baseof(ls, struct a_string, ls);
    a_string_free(s, self);
  }

  for (int i = 0; i < A_REG_COUNT; i++) {
    struct a_val *v = self->regs[i];
    if (v) { a_val_free(v, self); }
  }
  
  a_ls_do(&self->stack, vls) {
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_val_free(v, self);
  }

  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, ls);
    a_scope_deref(s);
    if (s != &self->main) { a_free(self, s); }
  }

  a_pool_deinit(&self->val_pool);
  a_pool_deinit(&self->type_pool);
  a_pool_deinit(&self->thread_pool);
  a_pool_deinit(&self->queue_pool);
  a_pool_deinit(&self->prim_pool);
  a_pool_deinit(&self->op_pool);
  a_pool_deinit(&self->iter_pool);
  a_pool_deinit(&self->multi_pool);
  a_pool_deinit(&self->func_pool);
  a_pool_deinit(&self->form_pool);

  a_lib_deinit(&self->math.lib);
  a_lib_deinit(&self->abc.lib);  
}

a_pc_t a_pc(struct a_vm *self) { return self->code.prev; }

struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type) {
  struct a_op *op = a_op_init(a_pool_alloc(&self->op_pool), op_type, self);
  a_ls_push(&self->code, &op->pc);
  return op;
}

bool a_analyze(struct a_vm *self, a_pc_t pc) {
  struct a_ls stack = self->stack;
  a_ls_init(&self->stack);
  bool ok = true;
  
  while (pc && pc != &self->code) {
    struct a_op *op = a_baseof(pc, struct a_op, pc);

    if (!(pc = a_op_analyze(op, self))) {
      ok = false;
      break;
    }
  }
  
  a_reset(self);
  self->stack = stack;
  return ok;
}

struct a_scope *a_scope(struct a_vm *self) {
  return a_baseof(self->scopes.prev, struct a_scope, ls);
}

struct a_scope *a_push_scope(struct a_vm *self, struct a_scope *scope) {
  a_ls_push(&self->scopes, &scope->ls);
  return scope;
}

struct a_scope *a_begin(struct a_vm *self) {
  struct a_scope *s = a_scope_init(a_malloc(self, sizeof(struct a_scope)), self, a_scope(self));
  return a_push_scope(self, s);
}

struct a_scope *a_end(struct a_vm *self) {
  struct a_ls *ls = self->scopes.prev;
  assert(ls != &self->scopes);
  a_ls_pop(ls);
  return a_baseof(ls, struct a_scope, ls);
}

struct a_frame *a_push_frame(struct a_vm *self,
			     struct a_func *func,
			     enum a_call_flags flags,
			     struct a_func_mem *mem,
			     a_pc_t ret) {
  assert(self->frame_count < A_FRAME_COUNT);
  return a_frame_init(self->frames + self->frame_count++, self, func, flags, mem, ret);
}

struct a_frame *a_pop_frame(struct a_vm *self) {
  assert(self->frame_count);
  struct a_frame *f = self->frames + --self->frame_count;
  a_frame_restore(f, self);
  return f;
}

void a_store(struct a_vm *self, a_reg_t reg, struct a_val *val) {
  struct a_val *prev = self->regs[reg];
  if (prev) { a_val_free(prev, self); }
  self->regs[reg] = val;
}

bool a_include(struct a_vm *self, const char *path) {
  FILE *in = fopen(path, "r");
  if (!in) { a_fail("Failed opening file: %d", errno); }

  struct a_parser parser;
  a_parser_init(&parser, self, a_string(self, path));
  while (!feof(in)) { a_stream_getline(&parser.in, in); }
  fclose(in);
  
  while (a_parser_next(&parser));
  struct a_form *f;
    
  while ((f = a_parser_pop(&parser))) {
    if (!a_form_emit(f, self)) { return false; }
    a_form_deref(f, self);
  }
  
  return true;
}

bool a_feval(struct a_vm *self, const char *path) {
  a_pc_t pc = a_pc(self);
  if (!a_include(self, path)) { return false; }  
  a_emit(self, A_STOP_OP);
  return a_analyze(self, pc) && a_eval(self, pc);
}

void *a_malloc(struct a_vm *vm, uint32_t size) { return malloc(size); }

void a_free(struct a_vm *vm, void *p) { free(p); }
