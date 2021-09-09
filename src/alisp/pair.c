#include <stdio.h>
#include "alisp/pair.h"
#include "alisp/val.h"

struct a_pair a_pair_new(struct a_val *left, struct a_val *right) {
  return (struct a_pair){.left = left, .right = right};
}

struct a_pair *a_pair_init(struct a_pair *self, struct a_val *left, struct a_val *right) {
  self->left = left;
  self->right = right;
  return self;
}

void a_pair_dump(struct a_pair *self) {
  a_dump(self->left);
  putc(':', stdout);
  a_dump(self->right);
}


