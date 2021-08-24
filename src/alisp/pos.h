#ifndef ALISP_POS_H
#define ALISP_POS_H

#include <stdint.h>

#define A_MIN_LINE 1
#define A_MIN_COLUMN 0

struct a_string;

struct a_pos {
  struct a_string *source;
  uint16_t line, column;
};

struct a_pos a_pos(struct a_string *source, uint16_t line, uint16_t column);
  
#endif
