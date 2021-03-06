#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/iter.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/thread.h"
#include "alisp/timer.h"
#include "alisp/vm.h"

/*
  #define A_TRACE(name)							\
  printf(#name "\n");							\
*/

#define A_TRACE(name)

#define A_DISPATCH(prev)						\
  goto *dispatch[a_baseof((pc = (prev)->next), struct a_op, pc)->type]

bool a_eval(struct a_vm *self, a_pc_t pc) {
  static const void* dispatch[] = {&&STOP,
    &&BENCH, &&BRANCH, &&BREAK,
    &&CALL,
    &&DROP, &&DUP,
    &&FENCE, &&FOR,
    &&GOTO, &&JOIN, &&LOAD, &&PUSH, &&QUOTE,
    &&RESET, &&RET,
    &&STORE, &&SWAP,
    &&TEST, &&THREAD,
    &&ZIP
  };
  
  A_DISPATCH(pc);

 BENCH: {
    A_TRACE(BENCH);
    struct a_bench_op *op = &a_baseof(pc, struct a_op, pc)->as_bench;
    struct a_ls stack = self->stack;
    a_ls_init(&self->stack);
    struct a_timer t;
    a_timer_reset(&t);
    
    for (int i = 0; i < op->reps; i++) {
      if (!a_eval(self, pc)) { return false; }
    }
    
    int msecs = a_timer_msecs(&t);
    a_reset(self);
    self->stack = stack;
    a_push(self, &self->abc.int_type)->as_int = msecs;
    A_DISPATCH(op->end_pc);
  }
  
 BRANCH: {
    A_TRACE(BRANCH);
    struct a_val *c = a_pop(self);

    if (c == NULL) {
      a_fail("Missing branch condition");
      return false;
    }

    A_DISPATCH(a_true(c) ? pc : a_baseof(pc, struct a_op, pc)->as_branch.right_pc);
  }

 BREAK: {
    A_TRACE(BREAK);
    self->break_depth++;
    A_DISPATCH(pc);    
  }
  
 CALL: {
    A_TRACE(CALL);
    struct a_call_op *call = &a_baseof(pc, struct a_op, pc)->as_call;
    struct a_val *t = call->target;
    if (t == NULL) { t = a_pop(self); }

    if (t == NULL) {
      a_fail("Missing call target");
      return false;
    }

    if (!(pc = a_call(t, call->flags, pc))) { return false; }
    if (!call->target) { a_val_free(t, self); }
    A_DISPATCH(pc);
  }

 DROP: {
    A_TRACE(DROP);
    struct a_drop_op *op = &a_baseof(pc, struct a_op, pc)->as_drop;
    int offset = op->offset;
    
    if (offset == -1) {
      struct a_val *v = a_pop(self);

      if (!v) {
	a_fail("Missing offset");
	return false;
      }

      if (v->type != &self->abc.int_type) {
	a_fail("Invalid offset: %s", v->type->name->data);
	return false;
      }

      offset = v->as_int;
    }

    int count = op->count;

    if (count == -1) {
      struct a_val *v = a_pop(self);

      if (!v) {
	a_fail("Missing count");
	return false;
      }

      if (v->type != &self->abc.int_type) {
	a_fail("Invalid drop count: %s", v->type->name->data);
	return false;
      }

      count = v->as_int;
    }

    if (!a_drop(self, offset, count)) { return false; }    
    A_DISPATCH(pc);    
  }

 DUP: {
    A_TRACE(SWAP);
    struct a_val *v = a_peek(self, a_baseof(pc, struct a_op, pc)->as_dup.offset);

    if (!v) {
      a_fail("Not enough values on stack");
      return false;
    }

    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);    
  }

 FENCE: {
    A_TRACE(FENCE);
    A_DISPATCH(pc);
  }

 FOR: {
    A_TRACE(FOR);
    struct a_val *in = a_pop(self);

    if (!a_isa(in->type, &self->abc.seq_type)) {
      a_fail("Invalid sequence: %s", in->type->name->data);
      return false;
    }

    struct a_iter *it = a_iter(in);
    struct a_val *v = NULL;

    while (!self->break_depth && (v = a_iter_next(it, self))) {
      a_ls_push(&self->stack, &v->ls);
      a_eval(self, pc);      
    }

    if (self->break_depth) { self->break_depth--; }
    
    a_val_free(in, self);
    a_iter_deref(it, self);
    A_DISPATCH(a_baseof(pc, struct a_op, pc)->as_for.end_pc);
  }
  
 GOTO: {
    A_TRACE(GOTO);
    A_DISPATCH(a_baseof(pc, struct a_op, pc)->as_goto.pc);
  }
  
 JOIN: {
    A_TRACE(JOIN);
    struct a_val *t = a_pop(self);

    if (!t) {
      a_fail("Missing thread");
      return false;
    }

    if (t->type != &self->abc.thread_type) {
      a_fail("Invalid thread: %s", t->type->name->data);
      return false;
    }

    if (!a_thread_join(t->as_thread)) { return false; }
    A_DISPATCH(pc);
  }

 LOAD: {
    A_TRACE(LOAD);
    struct a_val *v = self->regs[a_baseof(pc, struct a_op, pc)->as_load.reg];
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);
  }
  
 PUSH: {
    A_TRACE(PUSH);

    struct a_val
      *src = &a_baseof(pc, struct a_op, pc)->as_push.val,
      *dst = a_push(self, src->type);

    a_copy(dst, src);
    A_DISPATCH(pc);
  }

 QUOTE: {
    A_TRACE(QUOTE);
    struct a_form *f = a_baseof(pc, struct a_op, pc)->as_quote.form;
    struct a_val *v = a_form_quote(f, self);

    if (!v) {
      a_fail("Quote not supported: %d", f->type);
      return false;
    }
    
    a_ls_push(&self->stack, &v->ls);
    A_DISPATCH(pc);
  }

 RESET: {
    A_TRACE(RESET);
    a_reset(self);
    A_DISPATCH(pc);    
  }

 RET: {
    A_TRACE(RET);
    struct a_frame *f = a_pop_frame(self);

    if (!f) {
      a_fail("No calls in progress");
      return false;
    }

    struct a_ls *sp = self->stack.prev;
    
    for (struct a_type **r = f->func->rets.items+f->func->rets.count-1; r >= f->func->rets.items; r--, sp = sp->prev) {
      if (sp == &self->stack) {
	a_fail("Not enough return values on stack");
	return false;
      }
      
      struct a_type *rt = a_baseof(sp, struct a_val, ls)->type;
      
      if (!a_isa(rt, *r)) {
	a_fail("Invalid return value: %s", rt->name->data);
	return false;
      }
    }

    if (f->flags & A_CALL_MEM) { a_func_mem(f->func, self, f->mem); }
    if (f->flags & A_CALL_DRETS) { a_drop(self, 0, f->func->rets.count); }
    A_DISPATCH(f->ret);
  }
  
 STORE: {
    A_TRACE(STORE);
    struct a_val *v = a_pop(self);

    if (!v) {
      a_fail("Missing value to store");
      return false;
    }
    
    a_store(self, a_baseof(pc, struct a_op, pc)->as_load.reg, v); 
    A_DISPATCH(pc);
  }

 SWAP: {
    A_TRACE(SWAP);
    struct a_val *x = a_peek(self, a_baseof(pc, struct a_op, pc)->as_swap.offset + 1);
    
    if (!x) {
      a_fail("Not enough values on stack");
      return false;
    }

    struct a_val *y = a_peek(self, 0);
    a_ls_push(&x->ls, a_ls_pop(&y->ls));
    a_ls_push(&self->stack, a_ls_pop(&x->ls));
    A_DISPATCH(pc);    
  }

 TEST: {
    A_TRACE(BENCH);
    struct a_test_op *op = &a_baseof(pc, struct a_op, pc)->as_test;
    struct a_ls stack = self->stack;
    a_ls_init(&self->stack);
    printf("Testing %s...", op->desc->data);
    bool ok = true;
    if (!a_eval(self, pc)) { return false; }

    for (struct a_ls *exp = op->stack.prev, *act = self->stack.prev; exp != &op->stack; exp = exp->prev, act = act->prev) {
      if (act == &self->stack || !a_equals(a_baseof(act, struct a_val, ls), a_baseof(exp, struct a_val, ls))) {
	ok = false;
	break;
      }
    }

    if (ok) {
      printf("success!\n");
    } else {
      printf("failure!\n");
      printf("Expected: ");
      a_stack_dump(&op->stack);
      printf("\nActual:   ");
      a_stack_dump(&self->stack);
      putc('\n', stdout);
    }
    
    a_reset(self);
    self->stack = stack;
    A_DISPATCH(op->end_pc);
  }

 THREAD: {
    A_TRACE(THREAD);
    struct a_thread_op *op = &a_baseof(pc, struct a_op, pc)->as_thread;
    struct a_thread *t = a_thread_new(self, op->rets);

    if (!a_thread_emit(t, &op->args) || !a_thread_start(t)) {
      a_thread_deref(t);
      return false;
    }
    
    a_push(self, &self->abc.thread_type)->as_thread = t;
    A_DISPATCH(pc);
  }

 ZIP: {
    A_TRACE(ZIP);
    struct a_val *r = a_pop(self), *l = a_pop(self);

    if (!l || !r) {
      a_fail("Not enough values on stack");
      return false;
    }

    if (r->type == &self->abc.nil_type) {
      a_copy(a_push(self, l->type), l);
      a_val_free(l, self);
      a_val_free(r, self);
    } else {
      a_pair_init(&a_push(self, &self->abc.pair_type)->as_pair, l, r);
    }
    
    A_DISPATCH(pc);
  }
  
 STOP: {
    A_TRACE(STOP);
  }

  return true;
}
