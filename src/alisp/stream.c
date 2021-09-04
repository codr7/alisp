#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "alisp/fail.h"
#include "alisp/stream.h"

struct a_stream *a_stream_init(struct a_stream *self) {
  self->data = NULL;
  self->rpos = self->cap = self->length = 0;
  return self;
}

void a_stream_deinit(struct a_stream *self) {
  if (self->data) { free(self->data); }
}

void a_stream_reset(struct a_stream *self) {
  if (self->data) {
    self->rpos = self->length = 0;
    self->data[0] = 0;
  }
}

void a_stream_grow(struct a_stream *self, uint64_t length) {
  if (self->cap) {
    while (self->cap < length+1) {
      self->cap *= 2;
    }
  } else {
    self->cap = length + 1;
  }

  self->data = realloc(self->data, self->cap);
}

void a_stream_printf(struct a_stream *self, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  a_stream_vprintf(self, spec, args);
  va_end(args);
}

void a_stream_vprintf(struct a_stream *self, const char *spec, va_list args) {
  va_list len_args;
  va_copy(len_args, args);
  int len = vsnprintf(NULL, 0, spec, len_args);
  va_end(len_args);

  a_stream_grow(self, self->length + len);
  vsnprintf(self->data + self->length, len + 1, spec, args);
  self->length += len;
}

void a_stream_putc(struct a_stream *self, char c) {
  a_stream_grow(self, self->length+1);
  self->data[self->length++] = c;
  self->data[self->length] = 0;
}

char a_stream_getc(struct a_stream *self) {
  return (self->rpos < self->length) ? self->data[self->rpos++] : 0;
}

void a_stream_ungetc(struct a_stream *self) { self->rpos--; }

void a_stream_puts(struct a_stream *self, const char *s) {
  size_t sl = strlen(s);
  a_stream_grow(self, self->length+sl);
  strncpy(self->data + self->length, s, sl+1);
  self->length += sl;
}

char *a_stream_getline(struct a_stream *self, FILE *in) {
  uint64_t start = self->length;

  for (;;) {
    char c = fgetc(in);

    if (c == EOF) {
      if (ferror(in)) {
	a_fail("Failed reading char: %d", errno);
      } else {
	break;
      }
    }
    
    a_stream_putc(self, c);

    if (c == '\n') {
      break;
    }
  }

  return self->data ? self->data + start : NULL;
}
