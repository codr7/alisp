#include "alisp/val.h"

struct a_val *a_val_init(struct a_val *self, struct a_type *type) {
  self->type = type;
  return self;
}
