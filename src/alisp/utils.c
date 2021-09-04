#include <stdarg.h>
#include "alisp/stream.h"
#include "alisp/string.h"
#include "alisp/utils.h"

struct a_string *a_format(struct a_vm *vm, const char *spec, ...) {
  struct a_stream out;
  a_stream_init(&out);
  
  va_list args;
  va_start(args, spec);
  a_stream_vprintf(&out, spec, args);
  va_end(args);
  
  return a_string(vm, out.data);
}
