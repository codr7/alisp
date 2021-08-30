#include <errno.h>

#include "alisp/fail.h"
#include "alisp/timer.h"

void a_timer_reset(struct a_timer *self) {
  if (!timespec_get(&self->start, TIME_UTC)) {
    a_fail("Failed getting time: %d", errno);
  }
}

uint64_t a_timer_nsecs(const struct a_timer *self) {
  struct timespec end;
  
  if (!timespec_get(&end, TIME_UTC)) {
    a_fail("Failed getting time: %d", errno);
    return 0;
  }
  
  return
    (end.tv_sec - self->start.tv_sec) * 1000000000 +
    (end.tv_nsec - self->start.tv_nsec);
}

uint64_t a_timer_usecs(const struct a_timer *self) {
  return a_timer_nsecs(self) / 1000;
}

uint64_t a_timer_msecs(const struct a_timer *self) {
  return a_timer_usecs(self) / 1000;
}
