#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/func.h"
#include "alisp/libs/abc.h"
#include "alisp/multi.h"
#include "alisp/prim.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/utils.h"
#include "alisp/vm.h"

static a_pc_t equals_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_equals(x, y);
  a_val_free(x, vm);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t lt_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_compare(x, y) == A_LT;
  a_val_free(x, vm);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t gt_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_compare(x, y) == A_GT;
  a_val_free(x, vm);
  a_val_free(y, vm);
  return ret;
}

static bool bench_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args;
  struct a_form *reps_form = a_baseof((a = a->next), struct a_form, ls);
  struct a_val *reps = a_form_val(reps_form, vm);

  if (!reps) {
    a_fail("Invalid reps: %d", reps_form->type);
    return false;
  }

  if (reps->type != &vm->abc.int_type) {
    a_fail("Invalid reps: %s", reps->type->name->data);
    return false;
  }

  struct a_bench_op *op = &a_emit(vm, A_BENCH_OP)->as_bench;
  op->reps = reps->as_int;

  while ((a = a->next) != args) {
    if (!a_form_emit(a_baseof(a, struct a_form, ls), vm)) { return false; }
  }

  a_emit(vm, A_STOP_OP);
  op->end_pc = a_pc(vm);
  return true;
}

static bool ceval_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_goto_op *skip = &a_emit(vm, A_GOTO_OP)->as_goto;
  a_pc_t pc = a_pc(vm);

  a_ls_do(args, ls) {
    if (!a_form_emit(a_baseof(ls, struct a_form, ls), vm)) { return false; }
  }
  
  a_emit(vm, A_STOP_OP);
  skip->pc = a_pc(vm);
  if (!a_analyze(vm, pc)) { return false; }

  struct a_ls *sp = vm->stack.prev;
  if (!a_eval(vm, pc)) { return false; }
  struct a_ls *next = sp->next;
  
  while (next != &vm->stack) {
    struct a_val *v = a_baseof(a_ls_pop(sp = next), struct a_val, ls);
    next = sp->next;  
    a_copy(a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, v->type), v);
    a_val_free(v, vm);
  }
  
  return true;
}

static bool d_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args;
  int offset = 0;

  if ((a = a->next) != args) {
    struct a_form *f = a_baseof(a, struct a_form, ls);
    struct a_val *v = a_form_val(f, vm);

    if (v) {
      if (v->type != &vm->abc.int_type) {
	a_fail("Invalid offset: %s", v->type->name->data);
	return false;
      }

      offset = v->as_int;
    } else {
      a_form_emit(f, vm);
      offset = -1;
    }
  }
    
  int count = 1;
  
  if ((a = a->next) != args) {
    struct a_form *f = a_baseof(a, struct a_form, ls);
    struct a_val *v = a_form_val(f, vm);
    
    if (v) {
      if (v->type != &vm->abc.int_type) {
	a_fail("Invalid drop count: %s", v->type->name->data);
	return false;
      }
      
      count = v->as_int;
    } else {
      a_form_emit(f, vm);
      count = -1;
    }
  }
  
  struct a_drop_op *op = &a_emit(vm, A_DROP_OP)->as_drop;
  op->offset = offset;
  op->count = count;
  return true;
}

static bool def_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args;
  struct a_form *kf = a_baseof((a = a->next), struct a_form, ls);
  
  if (kf->type != A_ID_FORM) {
    a_fail("Invalid key form: %d", kf->type);
    return false;
  }
  
  struct a_string *k = kf->as_id.name;
  struct a_form *vf = a_baseof((a = a->next), struct a_form, ls);
  struct a_val *v = a_form_val(vf, vm);
  
  if (!v) {
    if (!a_form_eval(vf, vm)) { return false; }
    v = a_pop(vm);
  }      

  if (!v) {
    a_fail("Missing value");
    return false;
  }

  a_copy(a_scope_bind(a_scope(vm), k, v->type), v);
  return true;
}

static bool do_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_ls_do(args, a) {
    if (!a_form_emit(a_baseof(a, struct a_form, ls), vm)) { return false; }
  }

  return true;
}

static a_pc_t dump_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *v = a_pop(vm);
  a_dump(v);
  putc('\n', stdout);
  a_val_free(v, vm);
  return ret;
}

static bool dup_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_emit(vm, A_DUP_OP);
  return true;
}

static struct a_func *parse_func(struct a_ls *args, struct a_ls *a, struct a_string *name, struct a_vm *vm) {
  struct a_form *args_form = a_baseof(a, struct a_form, ls);

  if (args_form->type != A_LIST_FORM) {
    a_fail("Invalid function arguments: %d", args_form->type);
    return NULL;
  }

  struct a_form *rets_form = a_baseof((a = a->next), struct a_form, ls);

  if (rets_form->type != A_LIST_FORM) {
    a_fail("Invalid function returns: %d", rets_form->type);
    return NULL;
  }

  struct a_args fargs = A_ARG(vm);
  struct a_arg *fap = fargs.items;

  a_ls_do(&args_form->as_list.items, als) {
    struct a_form *af = a_baseof(als, struct a_form, ls);
    fap->name = NULL;
    
    if (af->type == A_PAIR_FORM) {
      struct a_form *nf = af->as_pair.left;

      if (nf->type != A_ID_FORM) {
	a_fail("Invalid argument name: %d", nf->type);
	return NULL;
      }
      
      fap->name = nf->as_id.name;
      af = af->as_pair.right;
    }

    if (af->type == A_ID_FORM) {
      struct a_val *v = a_scope_find(a_scope(vm), af->as_id.name);

      if (!v) {
	a_fail("Unknown argument type: %s", af->as_id.name->data);
	return NULL;
      }
	 
      if (v->type != &vm->abc.meta_type) {
	a_fail("Invalid argument type: %s", v->type->name->data);
	return NULL;
      }

      fap->type = v->as_meta;
    } else {
      a_fail("Invalid argument: %d", af->type);
      return NULL;
    }

    fap++;
    fargs.count++;
  }
  
  struct a_rets frets = A_RET(vm);
  struct a_type **frp = frets.items;
  
  a_ls_do(&rets_form->as_list.items, rls) {
    struct a_form *rf = a_baseof(rls, struct a_form, ls);
    
    if (rf->type != A_ID_FORM) {
      a_fail("Invalid return: %d", rf->type);
      return NULL;
    }

    struct a_val *v = a_scope_find(a_scope(vm), rf->as_id.name);

    if (!v) {
      a_fail("Unknown return type: %s", rf->as_id.name->data);
      return NULL;
    }

    if (v->type != &vm->abc.meta_type) {
      a_fail("Invalid return type: %s", v->type->name->data);
      return NULL;
    }
    
    *frp++ = v->as_meta;
    frets.count++;
  }
  
  struct a_func *f = a_func_new(vm, name, fargs, frets);

  if (name) {
    struct a_scope *s = a_scope(vm);
    struct a_val *v = a_scope_find(s, name);

    if (v) {
      if (v->type == &vm->abc.func_type) {
	a_val_init(v, &vm->abc.multi_type);
	v->as_multi = a_multi(vm, f->name, f->args.count);
	a_multi_add(v->as_multi, f);
      } else if (v->type == &vm->abc.multi_type) {
	a_multi_add(v->as_multi, f);      
      } else {
	a_fail("Invalid func binding: %s", v->type->name->data);
	return NULL;
      }
    } else {
      a_scope_bind(a_scope(vm), f->name, &vm->abc.func_type)->as_func = f;
    }
  }
  
  a_func_begin(f, vm);

  while ((a = a->next) != args) {
    struct a_form *f = a_baseof(a, struct a_form, ls);
    if (!a_form_emit(f, vm)) { return NULL; }
  }
  
  a_func_end(f, vm);
  return f;
}

static bool func_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args->next;
  struct a_form *name_form = a_baseof(a, struct a_form, ls);

  if (name_form->type != A_ID_FORM) {
    a_fail("Invalid function name: %d", name_form->type);
    return false;
  }

  return parse_func(args, a->next, name_form->as_id.name, vm);
}

static a_pc_t head_any_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  return ret;
}

static a_pc_t head_pair_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *p = a_pop(vm), *v = p->as_pair.left;
  a_copy(a_push(vm, v->type), v);
  a_val_free(p, vm);
  return ret;
}

static bool if_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args->next;
  struct a_form *cond = a_baseof(a, struct a_form, ls);
  if (!a_form_emit(cond, vm)) { return false; }

  struct a_branch_op *branch = &a_emit(vm, A_BRANCH_OP)->as_branch;
  struct a_form *left = a_baseof((a = a->next), struct a_form, ls), *right = NULL;
  if (!a_form_emit(left, vm)) { return false; }

  if ((a = a->next) != args) {
    struct a_goto_op *skip_right = &a_emit(vm, A_GOTO_OP)->as_goto;
    branch->right_pc = a_pc(vm);
    right = a_baseof(a, struct a_form, ls);
    if (!a_form_emit(right, vm)) { return false; }
    a_emit(vm, A_FENCE_OP);
    skip_right->pc = a_pc(vm);
  } else {
    a_emit(vm, A_FENCE_OP);
    branch->right_pc = a_pc(vm);
  }

  return true;
}

static bool include_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_ls_do(args, a) {
    struct a_form *f = a_baseof(a, struct a_form, ls);
    struct a_val *v = a_form_val(f, vm);

    if (!v || v->type != &vm->abc.string_type) {
      a_fail("Invalid path");
      return false;
    }

    a_include(vm, v->as_string->data);
  }

  return true;
}

static a_pc_t is_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_is(x, y);
  a_val_free(x, vm);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t iter_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *v = a_pop(vm);
  a_push(vm, &vm->abc.iter_type)->as_iter = a_iter(v);
  a_val_free(v, vm);
  return ret;
}

static bool lambda_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_func *f = parse_func(args, args->next, NULL, vm);
  if (!f) { return false; }
  f->name = a_format(vm, "%p", f);
  a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, &vm->abc.func_type)->as_func = f;
  return true;
}

static bool let_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_form *bsf = a_baseof(args->next, struct a_form, ls);

  if (bsf->type != A_LIST_FORM) {
    a_fail("Invalid bindings: %d", bsf->type);
    return false;
  }

  struct a_ls *bs = &bsf->as_list.items, *b = bs->prev;
  struct a_scope *s = a_begin(vm);

  while (b != bs) {
    struct a_form *vf = a_baseof(b, struct a_form, ls);

    if ((b = b->prev) == bs) {
      a_fail("Malformed bindings");
      a_end(vm);
      return false;
    }

    struct a_form *kf = a_baseof(b, struct a_form, ls);
    b = b->prev;    

    if (kf->type != A_ID_FORM) {
      a_fail("Invalid key form: %d", kf->type);
      a_end(vm);
      return false;
    }
    
    struct a_string *k = kf->as_id.name;
    struct a_val *v = a_form_val(vf, vm);
    
    if (v) {
      a_copy(a_scope_bind(s, k, v->type), v);
    } else {
      a_reg_t reg = a_scope_bind_reg(s, k);

      if (!a_form_emit(vf, vm)) {
	a_end(vm);
	return false;
      }
      
      a_emit(vm, A_STORE_OP)->as_store.reg = reg;
    }
  }

  for (struct a_ls *fls = args->next->next; fls != args; fls = fls->next) {
    if (!a_form_emit(a_baseof(fls, struct a_form, ls), vm)) { return false; }
  }

  a_end(vm);
  return true;
}

static a_pc_t nil_any_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *v = a_peek(vm, 0);
  a_deref(v);
  a_val_init(v, &vm->abc.bool_type)->as_bool = false; 
  return ret;
}

static a_pc_t nil_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *v = a_peek(vm, 0);
  a_val_init(v, &vm->abc.bool_type)->as_bool = true; 
  return ret;
}

static bool reset_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_emit(vm, A_RESET_OP);
  return true;
}

static a_pc_t reverse_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *lst = a_peek(vm, 0);
  if (lst->type == &vm->abc.nil_type || lst->as_pair.right->type == &vm->abc.nil_type) { return ret; }
  struct a_val *v = lst;
  int count = 0;

  for (;;) {
    if (v->type == &vm->abc.pair_type) {
      count++;
      v = v->as_pair.right;
    } else {
      if (v->type != &vm->abc.nil_type) { count++; }
      break;
    }
  }

  struct a_val **vs = a_malloc(vm, count*sizeof(struct a_val *)), **p = vs;
  v = lst;
  
  for (;;) {
    if (v->type == &vm->abc.pair_type) {
      *p++ = v->as_pair.left;
      v = v->as_pair.right;
    } else {
      if (v->type != &vm->abc.nil_type) { *p = v; }
      break;
    }
  }

  for (struct a_val **l = vs, **r = vs+count-1; l < r; l++, r--) {
    struct a_val tmp = **l;
    **l = **r;
    **r = tmp;
  }

  a_free(vm, vs);
  return ret;
}

static bool swap_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_emit(vm, A_SWAP_OP);
  return true;
}

static a_pc_t tail_any_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *v = a_pop(vm);
  a_push(vm, &vm->abc.nil_type);
  a_val_free(v, vm);
  return ret;
}

static a_pc_t tail_pair_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *p = a_pop(vm), *v = p->as_pair.right;
  a_copy(a_push(vm, v->type), v);
  a_val_free(p, vm);
  return ret;
}

static bool test_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args;
  struct a_form *desc_form = a_baseof((a = a->next), struct a_form, ls);
  struct a_val *desc = a_form_val(desc_form, vm);
  
  if (!desc) {
    a_fail("Missing description");
    return false;
  }

  if (desc->type != &vm->abc.string_type) {
    a_fail("Invalid description: %s", desc->type->name->data);
    return false;
  }
  
  struct a_form *stack_form = a_baseof((a = a->next), struct a_form, ls);
  struct a_val *stack = a_form_val(stack_form, vm);
  
  if (!stack) {
    a_fail("Missing stack");
    return false;
  }

  if (stack->type != &vm->abc.pair_type) {
    a_fail("Invalid stack: %s", stack->type->name->data);
    return false;
  }
  
  struct a_test_op *op = &a_emit(vm, A_TEST_OP)->as_test;
  op->desc = desc->as_string;
  
  struct a_val *src = stack;

  for (;;) {
    if (src->type == &vm->abc.pair_type) {
      struct a_val *dst = a_val_new(src->as_pair.left->type);
      a_copy(dst, src->as_pair.left);
      a_ls_push(&op->stack, &dst->ls);
      src = src->as_pair.right;
    } else {
      if (src->type != &vm->abc.nil_type) {
	struct a_val *dst = a_val_new(src->type);
	a_copy(dst, src);
	a_ls_push(&op->stack, &dst->ls);
      }
      
      break;
    }
  }
  
  while ((a = a->next) != args) {
    if (!a_form_emit(a_baseof(a, struct a_form, ls), vm)) { return false; }
  }

  a_emit(vm, A_STOP_OP);
  op->end_pc = a_pc(vm);
  return true;
}

static bool unbind_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_form *id = a_baseof(args->next, struct a_form, ls);

  if (id->type != A_ID_FORM) {
    a_fail("Invalid id: %d", id->type);
    return false;
  }

  a_scope_unbind(a_scope(vm), id->as_id.name);
  return true;
}

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm) {
  a_lib_init(&self->lib, vm, a_string(vm, "abc"));
  a_lib_bind_type(&self->lib, a_type_init(&self->any_type, vm, a_string(vm, "Any"), A_SUPER(NULL)));
  a_lib_bind_type(&self->lib, a_type_init(&self->num_type, vm, a_string(vm, "Num"), A_SUPER(&self->any_type)));
  a_lib_bind_type(&self->lib, a_type_init(&self->seq_type, vm, a_string(vm, "Seq"), A_SUPER(&self->any_type)));
  a_lib_bind_type(&self->lib, a_type_init(&self->target_type, vm, a_string(vm, "Target"), A_SUPER(&self->any_type)));

  a_lib_bind_type(&self->lib, a_type_init(&self->list_type, vm, a_string(vm, "List"),
					  A_SUPER(&self->seq_type)));

  a_lib_bind_type(&self->lib, a_bool_type_init(&self->bool_type, vm, a_string(vm, "Bool"),
					       A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_func_type_init(&self->func_type, vm, a_string(vm, "Func"),
					       A_SUPER(&self->target_type)));

  a_lib_bind_type(&self->lib, a_int_type_init(&self->int_type, vm, a_string(vm, "Int"),
					      A_SUPER(&self->num_type, &self->seq_type)));

  a_lib_bind_type(&self->lib, a_iter_type_init(&self->iter_type, vm, a_string(vm, "Iter"),
					       A_SUPER(&self->seq_type)));

  a_lib_bind_type(&self->lib, a_meta_type_init(&self->meta_type, vm, a_string(vm, "Meta"),
					       A_SUPER(&self->any_type)));

  a_lib_bind_type(&self->lib, a_multi_type_init(&self->multi_type, vm, a_string(vm, "Multi"),
						A_SUPER(&self->target_type)));

  a_lib_bind_type(&self->lib, a_nil_type_init(&self->nil_type, vm, a_string(vm, "Nil"),
					      A_SUPER(&self->list_type)));
  
  a_lib_bind_type(&self->lib, a_pair_type_init(&self->pair_type, vm, a_string(vm, "Pair"),
					       A_SUPER(&self->list_type)));

  a_lib_bind_type(&self->lib, a_reg_type_init(&self->reg_type, vm, a_string(vm, "Reg"),
					      A_SUPER(&self->any_type)));

  a_lib_bind_type(&self->lib, a_string_type_init(&self->string_type, vm, a_string(vm, "String"),
						 A_SUPER(&self->seq_type)));

  a_lib_bind_type(&self->lib, a_prim_type_init(&self->prim_type, vm, a_string(vm, "Prim"),
					       A_SUPER(&self->any_type)));

  a_lib_bind_type(&self->lib, a_type_init(&self->undef_type, vm, a_string(vm, "Undef"), A_SUPER(NULL)));

  a_lib_bind(&self->lib, a_string(vm, "ALISP-VERSION"), &self->int_type)->as_int = A_VERSION;
  a_lib_bind(&self->lib, a_string(vm, "NIL"), &self->nil_type);
  a_lib_bind(&self->lib, a_string(vm, "T"), &self->bool_type)->as_bool = true;
  a_lib_bind(&self->lib, a_string(vm, "F"), &self->bool_type)->as_bool = false;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "="),
			     A_ARG(vm,
				   {a_string(vm, "x"), &vm->abc.any_type},
				   {a_string(vm, "y"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = equals_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "<"),
			     A_ARG(vm,
				   {a_string(vm, "x"), &vm->abc.any_type},
				   {a_string(vm, "y"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = lt_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, ">"),
			     A_ARG(vm,
				   {a_string(vm, "x"), &vm->abc.any_type},
				   {a_string(vm, "y"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = gt_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "bench"), 1, -1))->body = bench_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "ceval"), 0, -1))->body = ceval_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "d"), 0, 2))->body = d_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "def"), 2, 2))->body = def_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "do"), 0, -1))->body = do_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "dump"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.any_type}),
			     A_RET(vm)))->body = dump_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "dup"), 0, 0))->body = dup_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "func"), 3, -1))->body = func_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "head"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.any_type)))->body = head_any_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "head"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.pair_type}),
			     A_RET(vm, &vm->abc.any_type)))->body = head_pair_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "if"), 2, 3))->body = if_body;
  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "include"), 0, -1))->body = include_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "is"),
			     A_ARG(vm,
				   {a_string(vm, "x"), &vm->abc.any_type},
				   {a_string(vm, "y"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = is_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "iter"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.seq_type}),
			     A_RET(vm, &vm->abc.iter_type)))->body = iter_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "lambda"), 2, -1))->body = lambda_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "let"), 1, -1))->body = let_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "nil?"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = nil_any_body;
  
  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "nil?"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.nil_type}),
			     A_RET(vm, &vm->abc.bool_type)))->body = nil_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "reset"), 0, 0))->body = reset_body;
			 
  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "reverse"),
			     A_ARG(vm, {a_string(vm, "lst"), &vm->abc.list_type}),
			     A_RET(vm, &vm->abc.list_type)))->body = reverse_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "swap"), 0, 0))->body = swap_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "tail"),
			     A_ARG(vm, {a_string(vm, "val"), &vm->abc.any_type}),
			     A_RET(vm, &vm->abc.nil_type)))->body = tail_any_body;

  a_lib_bind_func(&self->lib,
		  a_func_new(vm, a_string(vm, "tail"), A_ARG(vm, {a_string(vm, "val"), &vm->abc.pair_type}),
			     A_RET(vm, &vm->abc.any_type)))->body = tail_pair_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "test"), 2, -1))->body = test_body;

  a_lib_bind_prim(&self->lib, a_prim_new(vm, a_string(vm, "unbind"), 1, 1))->body = unbind_body;

  return self;
}
