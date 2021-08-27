#ifndef A_STREAM_H
#define A_STREAM_H

#include <stdint.h>
#include <stdio.h>

struct a_stream {
  uint32_t rpos, cap, length;
  char *data;
};

struct a_stream *a_stream_init(struct a_stream *self);
void a_stream_deinit(struct a_stream *self);

void a_stream_reset(struct a_stream *self);
void a_stream_grow(struct a_stream *self, uint64_t length);
void a_stream_printf(struct a_stream *self, const char *spec, ...);
void a_stream_putc(struct a_stream *self, char c);
char a_stream_getc(struct a_stream *self);
void a_stream_ungetc(struct a_stream *self);
void a_stream_puts(struct a_stream *self, const char *value);
char *a_stream_getline(struct a_stream *self, FILE *in);

#endif
