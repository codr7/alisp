#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/func.h"
#include "alisp/multi.h"
#include "alisp/op.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/thread.h"
#include "alisp/vm.h"

struct a_op *a_op_init(struct a_op *self, enum a_op_type type, struct a_vm *vm) {
  self->type = type;

  switch (self->type) {
  case A_CALL_OP:
    self->as_call.target = NULL;
    self->as_call.flags = 0;
    break;
  case A_BENCH_OP:
    self->as_bench.reps = 1;
    self->as_bench.end_pc = NULL;
    break;
  case A_BRANCH_OP:
    self->as_branch.right_pc = NULL;
    break;
  case A_DROP_OP:
    self->as_drop.offset = 0;
    self->as_drop.count = 1;
    break;
  case A_DUP_OP:
    self->as_dup.offset = 0;
    break;
  case A_FOR_OP:
    self->as_for.end_pc = NULL;
    break;
  case A_GOTO_OP:
    self->as_goto.pc = NULL;
    break;
  case A_LOAD_OP:
    self->as_load.reg = -1;
    self->as_load.type = NULL;
    break;
  case A_QUOTE_OP:
    self->as_quote.form = NULL;
    break;
  case A_RET_OP:
    self->as_ret.func = NULL;
    self->as_ret.check = true;
    break;
  case A_STORE_OP:
    self->as_store.reg = -1;
    break;
  case A_SWAP_OP:
    self->as_swap.offset = 0;
    break;
  case A_TEST_OP:
    self->as_test.desc = NULL;
    a_ls_init(&self->as_test.stack);
    self->as_test.end_pc = NULL;
    break;
  case A_THREAD_OP:
    a_ls_init(&self->as_thread.args);
    self->as_thread.rets = A_RET(vm);
    break;
  case A_BREAK_OP:
  case A_FENCE_OP:
  case A_JOIN_OP:
  case A_PUSH_OP:
  case A_RESET_OP:
  case A_STOP_OP:
  case A_ZIP_OP:
    break;
  }

  return self;
}

a_pc_t a_op_analyze(struct a_op *self, struct a_vm *vm) {  
  switch (self->type) {
  case A_BRANCH_OP: {
    a_drop(vm, 0, 1);
    break;
  }
        
  case A_CALL_OP: {
    struct a_call_op *op = &self->as_call;

    if (op->target) {
      if (op->target->type == &vm->abc.func_type) {
	struct a_func *f = op->target->as_func;

	if (op->flags & A_CALL_CHECK) {
	  struct a_ls *s = vm->stack.prev;
	  bool applicable = true;
	
	  for (struct a_arg *a = f->args.items + f->args.count-1; a >= f->args.items; a--, s = s->prev) {
	    if (s == &vm->stack) {
	      applicable = false;
	      break;
	    }
	  
	    struct a_val *sv = a_baseof(s, struct a_val, ls);

	    if (!a_isa(sv->type, a->type)) {
	      if (!sv->undef) {
		a_fail("Func not applicable: %s %s %s", f->name->data, sv->type->name->data, a->type->name->data);
		return NULL;
	      }
	    
	      applicable = false;
	      break;
	    }
	  }

	  if (applicable) {
	    op->flags ^= A_CALL_CHECK;
	    printf("Disabled arg check: %s ", f->name->data);
	    a_stack_type_dump(&vm->stack);
	    putc('\n', stdout);
	  }
	}

	a_drop(vm, 0, f->args.count);

	if (!(op->flags & A_CALL_DRETS)) {
	  for (struct a_type **rt = f->rets.items; rt < f->rets.items + f->rets.count; rt++) {
	    a_push(vm, *rt)->undef = true;
	  }
	}
      } else if (op->target->type == &vm->abc.multi_type) {
	uint8_t n = op->target->as_multi->arg_count;
	bool full = true;
	
	for (struct a_ls *v = vm->stack.prev; n; n--, v = v->prev) {
	  if (a_baseof(v, struct a_val, ls)->type == &vm->abc.undef_type) {
	    full = false;
	    break;
	  }
	}
	
	struct a_func *f = a_multi_specialize(op->target->as_multi, vm, full);
	
	if (f) {
	  a_deref(op->target);
	  a_val_init(op->target, &vm->abc.func_type)->as_func = f;
	  printf("Specialized multi: %s ", f->name->data);
	  a_stack_type_dump(&vm->stack);
	  putc('\n', stdout);
	}
      }
    } else {
      a_reset(vm);
    }
    
    break;
  }
    
  case A_DROP_OP: {
    a_drop(vm, self->as_drop.offset, self->as_drop.count);
    break;
  }

  case A_DUP_OP: {
    struct a_val *v = a_peek(vm, self->as_dup.offset);

    if (v) {
      a_copy(a_push(vm, v->type), v);
    } else {
      a_reset(vm);
    }
    
    break;
  }

  case A_JOIN_OP: {
    struct a_val *tv = a_pop(vm);
    
    if (tv) {
      if (!a_isa(&vm->abc.thread_type, tv->type)) {
	a_fail("Invalid thread: %s", tv->type->name->data);
	return NULL;
      }

      struct a_thread *t = tv->as_thread;
      for (struct a_type **r = t->rets.items; r < t->rets.items + t->rets.count; r++) { a_push(vm, *r)->undef = true; }
    } else {
      a_reset(vm);
    }

    break;
  }
    
  case A_PUSH_OP: {
    struct a_val *src = &self->as_push.val, *dst = a_push(vm, src->type);
    a_copy(dst, src);
    break;
  }

  case A_LOAD_OP: {
    a_push(vm, self->as_load.type ? self->as_load.type : &vm->abc.undef_type)->undef = true;
    break;
  }

  case A_QUOTE_OP: {
    struct a_val *v = a_form_quote(self->as_quote.form, vm);

    if (v) {
      a_ls_push(&vm->stack, &v->ls);
    } else {
      a_push(vm, &vm->abc.undef_type)->undef = true;
    }
    
    break;
  }
    
  case A_RET_OP: {
    struct a_ret_op *op = &self->as_ret;
    struct a_func *f = op->func;
    
    if (op->check) {
      struct a_ls *s = vm->stack.prev;
      bool ok = true;
	
      for (struct a_type **r = f->rets.items + f->rets.count-1; r >= f->rets.items; r--, s = s->prev) {
	if (s == &vm->stack) {
	  ok = false;
	  break;
	}
	
	struct a_val *sv = a_baseof(s, struct a_val, ls);
	
	if (!a_isa(sv->type, *r)) {
	  if (!sv->undef) {
	    a_fail("Invalid func result: %s %s", f->name->data, sv->type->name->data);
	    return NULL;
	  }
	  
	  ok = false;
	  break;
	}
      }

      if (ok) {
	op->check = false;
	printf("Disabled ret check: %s ", f->name->data);
	a_stack_type_dump(&vm->stack);
	putc('\n', stdout);
      }
    }

    a_reset(vm);
    break;
  }

  case A_STORE_OP: {
    a_drop(vm, 0, 1);
    break;
  }

  case A_SWAP_OP: {
    struct a_val *x = a_peek(vm, self->as_swap.offset+1);

    if (x) {
      struct a_val *y = a_peek(vm, 0);
      a_ls_push(&x->ls, a_ls_pop(&y->ls));
      a_ls_push(&vm->stack, a_ls_pop(&x->ls));
    } else {
      a_reset(vm);
    }
    
    break;
  }

  case A_ZIP_OP: {
    a_drop(vm, 0, 2);
    a_push(vm, &vm->abc.pair_type)->undef = true;
    break;
  }

  case A_BREAK_OP:
  case A_FENCE_OP:
  case A_GOTO_OP:
  case A_RESET_OP:
  case A_STOP_OP: {
    a_reset(vm);
    break;
  }
    
  case A_BENCH_OP:
  case A_FOR_OP:
  case A_TEST_OP:
  case A_THREAD_OP:
    break;
  }

  return self->pc.next;
}
