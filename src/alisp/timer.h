#ifndef ALISP_TIMER_H
#define ALISP_TIMER_H

#include <stdint.h>
#include <time.h>

struct a_timer {
  struct timespec start;
};

void a_timer_reset(struct a_timer *self);
uint64_t a_timer_nsecs(const struct a_timer *self);
uint64_t a_timer_usecs(const struct a_timer *self);
uint64_t a_timer_msecs(const struct a_timer *self);

#endif
