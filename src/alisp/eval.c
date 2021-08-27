#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/stack.h"
#include "alisp/vm.h"

#define A_DISPATCH(prev)						\
  goto *dispatch[a_baseof((pc = prev->next), struct a_op, ls)->type]

bool a_eval(struct a_vm *self, a_pc pc) {
  static const void* dispatch[] = {&&STOP, &&BRANCH, &&CALL, &&COPY, &&GOTO, &&LOAD, &&PUSH, &&STORE};
  A_DISPATCH(pc);

 BRANCH: {
    printf("BRANCH\n");
    struct a_val *c = a_pop(self);

    if (c == NULL) {
      a_fail("Missing branch condition");
      return false;
    }

    return a_true(c) ? pc : a_baseof(pc, struct a_op, ls)->as_branch.right_pc;
  }
  
 CALL: {
    printf("CALL\n");
    struct a_call_op *call = &a_baseof(pc, struct a_op, ls)->as_call;
    struct a_val *t = call->target;
    if (t == NULL) { t = a_pop(self); }

    if (t == NULL) {
      a_fail("Missing call target");
      return false;
    }

    pc = a_call(t, pc, true);

    if (!call->target) {
      a_val_deref(t);
      a_free(&self->val_pool, t);
    }
    
    A_DISPATCH(pc);
  }

 COPY: {
    printf("COPY\n");
    struct a_ls *vls = self->stack.prev;				   

    for (int i = a_baseof(pc, struct a_op, ls)->as_copy.offset; i >= 0; vls = vls->prev, i--) {
      if (vls == &self->stack) {
	a_fail("Not enough values on stack: %d", i+1);
	return false;
      }
    }

    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);    
  }
  
 GOTO: {
    printf("GOTO\n");
    A_DISPATCH(a_baseof(pc, struct a_op, ls)->as_goto.pc);
  }
  
 LOAD: {
    printf("LOAD\n");
    struct a_val *v = self->regs + a_baseof(pc, struct a_op, ls)->as_load.reg;
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);
  }
  
 PUSH: {
    printf("PUSH\n");

    struct a_val
      *src = &a_baseof(pc, struct a_op, ls)->as_push.val,
      *dst = a_push(self, src->type);

    a_copy(dst, src);
    A_DISPATCH(pc);
  }

 STORE: {
    printf("STORE\n");
    struct a_val *v = a_pop(self);
    if (!v) { a_fail("Missing value to store"); }
    self->regs[a_baseof(pc, struct a_op, ls)->as_load.reg] = *v;
    a_free(&self->val_pool, v);
    A_DISPATCH(pc);
  }
  
 STOP: {
    printf("STOP\n");
    // Done!
  }

  return true;
}
