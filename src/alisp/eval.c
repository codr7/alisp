#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

/*
  #define A_TRACE(name)							\
  printf(#name "\n");							\
*/

#define A_TRACE(name)

#define A_DISPATCH(prev)						\
  goto *dispatch[a_baseof((pc = (prev)->next), struct a_op, ls)->type]

bool a_eval(struct a_vm *self, a_pc pc) {
  static const void* dispatch[] = {
    &&STOP, &&BRANCH, &&CALL, &&COPY, &&DROP, &&GOTO, &&LOAD, &&PUSH, &&RESET, &&RET, &&STORE, &&ZIP
  };
  
  A_DISPATCH(pc);

 BRANCH: {
    A_TRACE(BRANCH);
    struct a_val *c = a_pop(self);

    if (c == NULL) {
      a_fail("Missing branch condition");
      return false;
    }

    A_DISPATCH(a_true(c) ? pc : a_baseof(pc, struct a_op, ls)->as_branch.right_pc);
  }
  
 CALL: {
    A_TRACE(CALL);
    struct a_call_op *call = &a_baseof(pc, struct a_op, ls)->as_call;
    struct a_val *t = call->target;
    if (t == NULL) { t = a_pop(self); }

    if (t == NULL) {
      a_fail("Missing call target");
      return false;
    }

    if (!(pc = a_call(t, call->flags, pc))) {
      return false;
    }

    if (!call->target) {
      a_val_deref(t);
      a_free(&self->val_pool, t);
    }
    
    A_DISPATCH(pc);
  }

 COPY: {
    A_TRACE(COPY);
    struct a_ls *vls = self->stack.prev;				   

    for (int i = a_baseof(pc, struct a_op, ls)->as_copy.offset; i > 0; vls = vls->prev, i--) {
      if (vls == &self->stack) {
	a_fail("Not enough values on stack: %d", i+1);
	return false;
      }
    }

    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);    
  }

 DROP: {
    A_TRACE(DROP);
    int count = a_baseof(pc, struct a_op, ls)->as_drop.count;

    if (count == -1) {
      struct a_val *v = a_pop(self);

      if (!v) {
	a_fail("Missing drop count");
	return false;
      }

      if (v->type != &self->abc.int_type) {
	a_fail("Invalid drop count: %s", v->type->name->data);
	return false;
      }

      count = v->as_int;
    }

    if (!a_drop(self, count)) { return false; }    
    A_DISPATCH(pc);    
  }
  
 GOTO: {
    A_TRACE(GOTO);
    A_DISPATCH(a_baseof(pc, struct a_op, ls)->as_goto.pc);
  }
  
 LOAD: {
    A_TRACE(LOAD);
    struct a_val *v = self->regs[a_baseof(pc, struct a_op, ls)->as_load.reg];
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);
  }
  
 PUSH: {
    A_TRACE(PUSH);

    struct a_val
      *src = &a_baseof(pc, struct a_op, ls)->as_push.val,
      *dst = a_push(self, src->type);

    a_copy(dst, src);
    A_DISPATCH(pc);
  }

 RESET: {
    A_TRACE(RESET);

    a_ls_do(&self->stack, ls) {
      struct a_val *v = a_baseof(ls, struct a_val, ls);
      a_val_deref(v);
      a_free(&self->val_pool, v);
    }

    a_ls_init(&self->stack);
    A_DISPATCH(pc);    
  }

 RET: {
    A_TRACE(RET);
    struct a_ls *fls = self->frames.prev;

    if (fls == &self->frames) {
      a_fail("No calls in progress");
      return false;
    }
    
    a_end(self);
    struct a_frame *f = a_baseof(a_ls_pop(fls), struct a_frame, ls);
    pc = a_frame_restore(f, self);
    a_frame_deinit(f);
    a_free(&self->frame_pool, f);
    A_DISPATCH(pc);
  }
  
 STORE: {
    A_TRACE(STORE);
    struct a_val *v = a_pop(self);
    if (!v) { a_fail("Missing value to store"); }
    self->regs[a_baseof(pc, struct a_op, ls)->as_load.reg] = v;
    A_DISPATCH(pc);
  }

 ZIP: {
    A_TRACE(ZIP);
    struct a_val *r = a_pop(self), *l = a_pop(self);

    if (!l || !r) {
      a_fail("Not enough values on stack");
      return false;
    }

    a_pair_init(&a_push(self, &self->abc.pair_type)->as_pair, l, r);
    A_DISPATCH(pc);
  }
  
 STOP: {
    A_TRACE(STOP);
  }

  return true;
}
