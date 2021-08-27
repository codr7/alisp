#include <assert.h>
#include <ctype.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/prim.h"
#include "alisp/string.h"
#include "alisp/vm.h"

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
  case A_LITERAL_FORM:
  case A_NOP_FORM:
    break;
  }

  return self;
}

struct a_form *a_form_ref(struct a_form *self) {
  self->ref_count++;
  return self;
}

bool a_form_deref(struct a_form *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }  

  switch (self->type) {
  case A_CALL_FORM:
    a_form_deref(self->as_call.target);
    a_ls_do(&self->as_call.args, als) { a_form_deref(a_baseof(als, struct a_form, ls)); }
    break;
  case A_LITERAL_FORM:
    a_val_deref(&self->as_literal.val);
    break;
  case A_ID_FORM:
  case A_NOP_FORM:
    break;
  }

  return true;
}

struct a_val *a_form_val(struct a_form *self, struct a_vm *vm) {
  switch (self->type) {
  case A_LITERAL_FORM:
    return &self->as_literal.val;
  case A_ID_FORM: {
    struct a_val *v = a_scope_find(a_scope(vm), self->as_id.name);
    if (v != NULL && v->type != &vm->abc.reg_type) { return v; }
    break;
  }
  case A_CALL_FORM:
  case A_NOP_FORM:
    break;
  }

  return NULL;
}

bool a_form_emit(struct a_form *self, struct a_vm *vm) {
  switch (self->type) {
  case A_CALL_FORM: {
    if (self->as_call.target->type != A_ID_FORM) {
      a_fail("Invalid call target");
      return false;
    }
    
    struct a_val *t = a_scope_find(a_scope(vm), self->as_call.target->as_id.name);

    if (!t) {
      a_fail("Unknown call target: %s", self->as_call.target->as_id.name->data);
      return false;
    }

    if (t->type == &vm->abc.prim_type) {
      return a_prim_call(t->as_prim, vm, &self->as_call.args, self->as_call.arg_count); 
    } else {
      a_ls_do(&self->as_call.args, als) {
	if (!a_form_emit(a_baseof(als, struct a_form, ls), vm)) { return false; }
      }

      if (t->type == &vm->abc.reg_type) {
	a_emit(vm, A_LOAD_OP)->as_load.reg = t->as_reg;
	t = NULL;
      }
      
      struct a_call_op *call = &a_emit(vm, A_CALL_OP)->as_call;

      if (t) {
	call->target = a_malloc(&vm->val_pool, sizeof(struct a_val));
	a_copy(call->target, t);
      }
    }
    
    break;
  }

  case A_ID_FORM: {
    struct a_string *id = self->as_id.name;

    if (id->length == 2 && id->data[0] == '$' && isdigit(id->data[1])) {
      a_emit(vm, A_COPY_OP)->as_copy.offset = id->data[1] - '0';
      break;
    }
    
    struct a_val *v = a_scope_find(a_scope(vm), id);

    if (v == NULL) {
      a_fail("Unknown symbol: %s", id->data);
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

  case A_LITERAL_FORM:
    a_copy(&a_emit(vm, A_PUSH_OP)->as_push.val, &self->as_literal.val);
    break;

  case A_NOP_FORM:
    break;
  }

    
  return true;
}

