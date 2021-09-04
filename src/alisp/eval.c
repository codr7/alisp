#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
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
    &&BENCH, &&BRANCH, &&CALL, &&DROP, &&DUP, &&FENCE, &&GOTO, &&LOAD, &&PUSH, &&RESET, &&RET, &&STORE, &&SWAP, &&ZIP
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

    if (!call->target) {
      a_val_deref(t);
      a_val_free(t, self);
    }
    
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
  
 GOTO: {
    A_TRACE(GOTO);
    A_DISPATCH(a_baseof(pc, struct a_op, pc)->as_goto.pc);
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
    
    for (struct a_type **r = f->func->rets->items+f->func->rets->count-1; r >= f->func->rets->items; r--, sp = sp->prev) {
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
    if (f->flags & A_CALL_DRETS) { a_drop(self, 0, f->func->rets->count); }
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

 ZIP: {
    A_TRACE(ZIP);
    struct a_val *r = a_pop(self), *l = a_pop(self);

    if (!l || !r) {
      a_fail("Not enough values on stack");
      return false;
    }

    if (r->type == &self->abc.nil_type) {
      a_copy(a_push(self, l->type), l);
      a_val_deref(l);
      a_val_free(l, self);
      a_val_deref(r);
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
