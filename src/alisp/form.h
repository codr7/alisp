#ifndef ALISP_FORM_H
#define ALISP_FORM_H

#include "alisp/ls.h"
#include "alisp/pos.h"
#include "alisp/types.h"
#include "alisp/val.h"

struct a_form;
struct a_string;
struct a_vm;

enum a_form_type {A_CALL_FORM, A_ID_FORM, A_LIST_FORM, A_LIT_FORM, A_NOP_FORM, A_PAIR_FORM, A_QUOTE_FORM};
 
struct a_call_form {
  struct a_form *target;
  struct a_ls args;
};

struct a_id_form {
  struct a_string *name;
};

struct a_list_form {
  struct a_ls items;
  struct a_val *val;
};

struct a_lit_form {
  struct a_val val;
};

struct a_pair_form {
  struct a_form *left, *right;
  struct a_val *val;
};

struct a_quote_form {
  struct a_form *form;
};

struct a_form {
  struct a_ls ls;
  enum a_form_type type;
  struct a_pos pos;
  a_ref_count_t ref_count;
  
  union {
    struct a_call_form as_call;
    struct a_id_form as_id;
    struct a_lit_form as_lit;
    struct a_list_form as_list;
    struct a_pair_form as_pair;
    struct a_quote_form as_quote;
  };
};

struct a_form *a_form_new(struct a_vm *vm, enum a_form_type type, struct a_pos pos);

struct a_form *a_form_init(struct a_form *self, enum a_form_type type, struct a_pos pos);
struct a_form *a_form_ref(struct a_form *self);
bool a_form_deref(struct a_form *self, struct a_vm *vm);

struct a_val *a_form_val(struct a_form *self, struct a_vm *vm);
bool a_form_emit(struct a_form *self, struct a_vm *vm);
bool a_form_eval(struct a_form *self, struct a_vm *vm);
struct a_form *a_form_clone(struct a_form *self, struct a_vm *dst_vm, struct a_vm *src_vm);
struct a_val *a_form_quote(struct a_form *self, struct a_vm *vm);

#endif
