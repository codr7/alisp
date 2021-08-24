#ifndef ALISP_FORM_H
#define ALISP_FORM_H

#include "alisp/pos.h"

struct a_string;

struct a_form_type {
  
};
  
struct a_call_form {
};

struct a_id_form {
};

struct a_literal_form {
};

struct a_form {
  struct a_form_type *type;
  struct a_pos pos;

  union {
    struct a_call_form as_call;
    struct a_id_form as_id;
    struct a_literal_form as_literal;
  };
};

struct a_form *a_form_init(struct a_form *self, struct a_form_type *type, struct a_pos pos);
  
#endif
