#include <assert.h>
#include <ctype.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/prim.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_form *a_form_new(struct a_vm *vm, enum a_form_type type, struct a_pos pos) {
  struct a_form *f = a_ls_null(&vm->free_forms)
    ? a_pool_alloc(&vm->form_pool)
    : a_baseof(a_ls_pop(vm->free_forms.next), struct a_form, ls);
  
  return a_form_init(f, type, pos);
}

struct a_form *a_form_init(struct a_form *self, enum a_form_type type, struct a_pos pos) {
  self->type = type;
  self->pos = pos;
  self->ref_count = 1;

  switch (self->type) {
  case A_CALL_FORM:
    self->as_call.target = NULL;
    a_ls_init(&self->as_call.args);
    break;
  case A_ID_FORM:
    self->as_id.name = NULL;
    break;
  case A_LIST_FORM:
    a_ls_init(&self->as_list.items);
    self->as_list.val = NULL;
    break;
  case A_PAIR_FORM:
    self->as_pair.left = self->as_pair.right = NULL;
    self->as_pair.val = NULL;
    break;
  case A_QUOTE_FORM:
    self->as_quote.form = NULL;
    break;
  case A_LIT_FORM:
  case A_NOP_FORM:
    break;
  }

  return self;
}

struct a_form *a_form_ref(struct a_form *self) {
  self->ref_count++;
  return self;
}

bool a_form_deref(struct a_form *self, struct a_vm *vm) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }  

  switch (self->type) {
  case A_CALL_FORM:
    a_form_deref(self->as_call.target, vm);
    a_ls_do(&self->as_call.args, ls) { a_form_deref(a_baseof(ls, struct a_form, ls), vm); }
    break;
  case A_LIT_FORM:
    a_deref(&self->as_lit.val);
    break;

  case A_LIST_FORM: {
    a_ls_do(&self->as_list.items, ls) { a_form_deref(a_baseof(ls, struct a_form, ls), vm); }
    if (self->as_list.val) { a_val_free(self->as_list.val, vm); }
    break;
  }
    
  case A_PAIR_FORM: {
    a_form_deref(self->as_pair.left, vm);
    a_form_deref(self->as_pair.right, vm);
    struct a_val *v = self->as_pair.val;
    if (v) { a_val_free(v, vm); }
    break;
  }

  case A_QUOTE_FORM: {
    a_form_deref(self->as_quote.form, vm);
    break;
  }

  case A_ID_FORM:
  case A_NOP_FORM:
    break;
  }

  a_ls_push(vm->free_forms.next, &self->ls);
  return true;
}

struct a_val *a_form_val(struct a_form *self, struct a_vm *vm) {
  switch (self->type) {
  case A_LIT_FORM:
    return &self->as_lit.val;

  case A_ID_FORM: {
    struct a_val *v = a_scope_find(a_scope(vm), self->as_id.name);
    if (v != NULL && v->type != &vm->abc.reg_type) { return v; }
    break;
  }
    
  case A_LIST_FORM: {
    if (self->as_list.val) { return self->as_list.val; }
    bool lit = true;

    a_ls_do(&self->as_list.items, ls) {
      if (!a_form_val(a_baseof(ls, struct a_form, ls), vm)) {
	lit = false;
	break;
      }
    }

    if (lit) {
      struct a_val *out = a_val_new(&vm->abc.pair_type), *v = out;
      v->as_pair.left = NULL;
      v->as_pair.right = NULL;
      
      a_ls_do(&self->as_list.items, ls) {
	struct a_val *src = a_form_val(a_baseof(ls, struct a_form, ls), vm);
	struct a_val *dst = a_val_new(src->type);
	a_copy(dst, src);

	if (ls->next == &self->as_list.items) {
	  if (v->as_pair.left) {
	    v->as_pair.right = dst;
	  } else {
	    v->as_pair.left = dst;
	  }
	} else if (v->as_pair.left) {
	  v = v->as_pair.right = a_val_new(&vm->abc.pair_type);
	  v->as_pair.left = dst;
	  v->as_pair.right = NULL;
	} else {
	  v->as_pair.left = dst;
	}
      }

      if (!v->as_pair.left && !v->as_pair.right) {
	out->type = &vm->abc.nil_type;
      } else if (!v->as_pair.right) {
	v->as_pair.right = a_val_new(&vm->abc.nil_type);
      }
      
      return out;
    }

    break;
  }

  case A_PAIR_FORM: {
    if (self->as_pair.val) { return self->as_pair.val; }
    
    struct a_val
      *lv = a_form_val(self->as_pair.left, vm),
      *rv = a_form_val(self->as_pair.right, vm);

    if (lv && rv) {
      struct a_val *v = a_val_new(&vm->abc.pair_type);
      v->as_pair = a_pair_new(lv, rv);
      self->as_pair.val = v;
      return v;
    }
    
    break;
  }

  case A_QUOTE_FORM:
    return a_form_quote(self, vm);
    
  case A_CALL_FORM:
  case A_NOP_FORM:
    break;
  }

  return NULL;
}

bool a_form_emit(struct a_form *self, struct a_vm *vm) {
  switch (self->type) {
  case A_CALL_FORM: {
    struct a_form *tf = self->as_call.target;
    enum a_call_flags flags = 0;

    if (tf->type == A_PAIR_FORM) {
      struct a_form *ff = tf->as_pair.right;
      tf = tf->as_pair.left;

      while (ff) {
	struct a_form *r = ff;
	
	if (r->type == A_PAIR_FORM) {
	  r = ff->as_pair.left;
	  ff = ff->as_pair.right;
	} else {
	  ff = NULL;
	}
		
	if (r->type == A_ID_FORM && strcmp(r->as_id.name->data, "d") == 0) {
	  flags |= A_CALL_DRETS;
	} else if (r->type == A_ID_FORM && strcmp(r->as_id.name->data, "m") == 0) {
	  flags |= A_CALL_MEM;
	} else if (r->type == A_ID_FORM && strcmp(r->as_id.name->data, "t") == 0) {
	  flags |= A_CALL_TCO;
	} else {
	  a_fail("Invalid call flag: %d", r->type);
	}	
      }
    }

    if (tf->type != A_ID_FORM && tf->type != A_NOP_FORM) {
      a_fail("Invalid call target: %d", tf->type);
      return false;
    }

    struct a_val *t = NULL;

    if (tf->type == A_ID_FORM) {
      struct a_string *id = tf->as_id.name;
      t = a_scope_find(a_scope(vm), id);

      if (!t) {
	a_fail("Unknown call target: %s", id->data);
	return false;
      }
    }

    if (t && t->type == &vm->abc.prim_type) {
      if (flags) {
	a_fail("Primitives don't support call flags: %d", flags);
	return false;
      }
      
      return a_prim_call(t->as_prim, vm, &self->as_call.args); 
    } else {
      a_ls_do(&self->as_call.args, als) {
	if (!a_form_emit(a_baseof(als, struct a_form, ls), vm)) { return false; }
      }

      if (t && t->type == &vm->abc.reg_type) {
	a_emit(vm, A_LOAD_OP)->as_load.reg = t->as_reg;
	t = NULL;
      }
      
      struct a_call_op *call = &a_emit(vm, A_CALL_OP)->as_call;
      if (t) { call->target = a_copy(a_val_new(t->type), t); }
      call->flags = A_CALL_CHECK | flags;
    }
    
    break;
  }

  case A_ID_FORM: {
    struct a_string *id = self->as_id.name;
    struct a_val *v = a_scope_find(a_scope(vm), id);

    if (v == NULL) {
      a_fail("Unknown id: %s", id->data);
      return false;
    }
    
    if (v->type == &vm->abc.reg_type) {
      a_emit(vm, A_LOAD_OP)->as_load.reg = v->as_reg;
    } else {
      struct a_val *dst = a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, v->type);
      a_copy(dst, v);
    }

    break;
  }

  case A_LIT_FORM: {
    struct a_val *v = &self->as_lit.val;
    a_copy(a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, v->type), v);
    break;
  }

  case A_LIST_FORM: {
    struct a_val *v = a_form_val(self, vm);

    if (v) {
      a_copy(a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, v->type), v);
    } else {
      a_fail("Dynamic ls emit is not implemented yet");
    }
    
    break;
  }

  case A_PAIR_FORM: {
    struct a_val *v = self->as_pair.val;

    if (v) {
      a_copy(a_val_init(&a_emit(vm, A_PUSH_OP)->as_push.val, v->type), v);
      break;
    } 
    
    if (!a_form_emit(self->as_pair.left, vm)) { return false; }
    if (!a_form_emit(self->as_pair.right, vm)) { return false; }
    a_emit(vm, A_ZIP_OP);
    break;
  }  

  case A_QUOTE_FORM: {
    a_emit(vm, A_QUOTE_OP)->as_quote.form = a_form_ref(self);
    break;
  }
    
  case A_NOP_FORM:
    break;
  }

    
  return true;
}

bool a_form_eval(struct a_form *self, struct a_vm *vm) {
  struct a_goto_op *skip = &a_emit(vm, A_GOTO_OP)->as_goto;
  a_pc_t pc = a_pc(vm);
  if (!a_form_emit(self, vm)) { return false; }
  a_emit(vm, A_STOP_OP);
  skip->pc = a_pc(vm);
  if (!a_analyze(vm, pc)) { return false; }
  if (!a_eval(vm, pc)) { return false; }
  return true;
}

struct a_form *a_form_clone(struct a_form *self, struct a_vm *dst_vm, struct a_vm *src_vm) {
  struct a_form *f = a_form_new(dst_vm, self->type, self->pos);

  switch (self->type) {
  case A_CALL_FORM: {
    struct a_call_form *src = &self->as_call, *dst = &f->as_call;
    dst->target = a_form_clone(src->target, dst_vm, src_vm);
    a_ls_do(&src->args, ls) { a_ls_push(&dst->args, &a_form_clone(a_baseof(ls, struct a_form, ls), dst_vm, src_vm)->ls); }
    break;
  }

  case A_ID_FORM: {
    struct a_id_form *src = &self->as_id, *dst = &f->as_id;
    dst->name = a_string(dst_vm, src->name->data);
    break;
  }

  case A_LIST_FORM: {
    struct a_list_form *src = &self->as_list, *dst = &f->as_list;

    a_ls_do(&src->items, ls) {
      a_ls_push(&dst->items, &a_form_clone(a_baseof(ls, struct a_form, ls), dst_vm, src_vm)->ls);
    }
    
    break;
  }

  case A_LIT_FORM: {
    struct a_lit_form *src = &self->as_lit, *dst = &f->as_lit;
    a_clone(a_val_init(&dst->val, a_type_clone(src->val.type, dst_vm)), &src->val);
    break;
  }

  case A_PAIR_FORM: {
    struct a_pair_form *src = &self->as_pair, *dst = &f->as_pair;
    dst->left = a_form_clone(src->left, dst_vm, src_vm);
    dst->right = a_form_clone(src->right, dst_vm, src_vm);
    break;
  }

  case A_QUOTE_FORM: {
    struct a_quote_form *src = &self->as_quote, *dst = &f->as_quote;
    dst->form = a_form_clone(src->form, dst_vm, src_vm);
    break;
  }

  case A_NOP_FORM:
    break;
  }

  return f;
}

struct a_val *a_form_quote(struct a_form *self, struct a_vm *vm) {
  switch (self->type) {
  case A_ID_FORM: {
    struct a_val *v = a_val_new(&vm->abc.sym_type);
    v->as_sym = self->as_id.name;
    return v;
  }

  case A_QUOTE_FORM:
    return a_form_quote(self->as_quote.form, vm);

  case A_CALL_FORM:
  case A_LIST_FORM:
  case A_LIT_FORM:
  case A_NOP_FORM:
  case A_PAIR_FORM:
    break;
  }

  return NULL;
}

