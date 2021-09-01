#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/op.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_op *a_op_init(struct a_op *self, enum a_op_type type) {
  self->type = type;

  switch (self->type) {
  case A_CALL_OP:
    self->as_call.flags = 0;
    break;
  case A_BENCH_OP:
  case A_BRANCH_OP:
  case A_COPY_OP:
    self->as_copy.offset = 0;
    break;
  case A_DROP_OP:
    self->as_drop.count = 1;
    break;
  case A_LOAD_OP:
    self->as_load.type = NULL;
    break;
  case A_RET_OP:
    self->as_ret.func = NULL;
    self->as_ret.check = true;
    break;
  case A_FENCE_OP:
  case A_GOTO_OP:
  case A_PUSH_OP:
  case A_RESET_OP:
  case A_STOP_OP:
  case A_STORE_OP:
  case A_ZIP_OP:
    break;
  }

  return self;
}

void a_op_deinit(struct a_op *self) {
  switch (self->type) {
  case A_CALL_OP: {
    struct a_val *t = self->as_call.target;
    if (t) { a_val_deref(t); }
    break;
  }
  case A_PUSH_OP:
    a_val_deref(&self->as_push.val);
    break;
  case A_BENCH_OP:
  case A_BRANCH_OP:
  case A_COPY_OP:
  case A_DROP_OP:
  case A_FENCE_OP:
  case A_GOTO_OP:
  case A_LOAD_OP:
  case A_STOP_OP:
  case A_RESET_OP:
  case A_RET_OP:
  case A_STORE_OP:
  case A_ZIP_OP:
    break;
  }
}

a_pc_t a_op_analyze(struct a_op *self, struct a_vm *vm) {  
  switch (self->type) {
  case A_BRANCH_OP: {
    a_drop(vm, 1);
    break;
  }
        
  case A_CALL_OP: {
    struct a_call_op *op = &self->as_call;

    if (op->target && op->target->type == &vm->abc.func_type) {
      struct a_func *f = op->target->as_func;

      if (op->flags & A_CALL_CHECK) {
	struct a_ls *s = vm->stack.prev;
	bool applicable = true;
	
	for (struct a_arg *a = f->args->items + f->args->count-1; a >= f->args->items; a--, s = s->prev) {
	  if (s == &vm->stack) {
	    applicable = false;
	    break;
	  }
	  
	  struct a_val *sv = a_baseof(s, struct a_val, ls);

	  if (!a_isa(sv->type, a->type)) {
	    if (sv->type != &vm->abc.undef_type) {
	      a_fail("Func not applicable: %s", f->name->data);
	      return NULL;
	    }
	    
	    applicable = false;
	    break;
	  }
	}

	if (applicable) {
	  op->flags ^= A_CALL_CHECK;
	  printf("Disabled arg check: %s %d ", f->name->data, op->flags);
	  a_stack_type_dump(&vm->stack);
	  putc('\n', stdout);
	}
      }

      a_drop(vm, f->args->count);

      if (!(op->flags & A_CALL_DRETS)) {
	for (struct a_type **rt = f->rets->items; rt < f->rets->items + f->rets->count; rt++) {
	  a_push(vm, *rt)->undef = true;
	}
      }
    } else {
      a_reset(vm);
    }
    
    break;
  }

  case A_COPY_OP: {
    struct a_val *v = a_peek(vm, self->as_copy.offset);

    if (v) {
      a_copy(a_push(vm, v->type), v);
    } else {
      a_reset(vm);
    }
    
    break;
  }
    
  case A_DROP_OP: {
    a_drop(vm, self->as_drop.count);
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

  case A_RET_OP: {
    struct a_ret_op *op = &self->as_ret;
    struct a_func *f = op->func;
    
    if (op->check) {
      struct a_ls *s = vm->stack.prev;
      bool ok = true;
	
      for (struct a_type **r = f->rets->items + f->rets->count-1; r >= f->rets->items; r--, s = s->prev) {
	if (s == &vm->stack) {
	  ok = false;
	  break;
	}
	
	struct a_val *sv = a_baseof(s, struct a_val, ls);
	
	if (!a_isa(sv->type, *r)) {
	  if (sv->type != &vm->abc.undef_type) {
	    a_fail("Invalid func result: %s %s", f->name->data, sv->type->name->data);
	    return NULL;
	  }
	  
	  ok = false;
	  break;
	}
      }

      if (ok) {
	op->check = false;
	printf("Disabled ret check: %s", f->name->data);
	a_stack_type_dump(&vm->stack);
	putc('\n', stdout);
      }
    }

    a_reset(vm);
    break;
  }

  case A_STORE_OP: {
    a_drop(vm, 1);
    break;
  }

  case A_ZIP_OP: {
    a_drop(vm, 2);
    a_push(vm, &vm->abc.pair_type)->undef = true;
    break;
  }

    
  case A_FENCE_OP:
  case A_GOTO_OP:
  case A_RESET_OP:
  case A_STOP_OP: {
    a_reset(vm);
    break;
  }
    
  case A_BENCH_OP:
    break;
  }

  return self->pc.next;
}
