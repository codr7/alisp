#ifndef ALISP_PAIR_H
#define ALISP_PAIR_H

struct a_val;

struct a_pair {
  struct a_val *left, *right;
};

struct a_pair a_pair(struct a_val *left, struct a_val *right);
struct a_pair *a_pair_init(struct a_pair *self, struct a_val *left, struct a_val *right);
void a_pair_dump(struct a_pair *self);

#endif
