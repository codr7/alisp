#include "alisp/pos.h"

struct a_pos a_pos(struct a_string *source, uint16_t line, uint16_t column) {
  return (struct a_pos){.source=source, .line=line, .column=column};
}
