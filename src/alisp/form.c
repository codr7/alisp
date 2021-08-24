#include "alisp/form.h"

struct a_form *a_form_init(struct a_form *self, struct a_form_type *type, struct a_pos pos) {
  self->type = type;
  self->pos = pos;
  return self;
}
