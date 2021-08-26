#ifndef ALISP_MACROS_H
#define ALISP_MACROS_H

#include <stddef.h>

#define A_MAX_ALIGN _Alignof(max_align_t)

#define a_align(base, size) ({					\
      __auto_type _base = base;					\
      __auto_type _size = a_min((size), A_MAX_ALIGN);		\
      (_base) + _size - ((ptrdiff_t)(_base)) % _size;		\
    })								\

#define a_baseof(p, t, m) ({			\
      uint8_t *_p = (uint8_t *)(p);		\
      _p ? ((t *)(_p - offsetof(t, m))) : NULL;	\
    })

#define _a_concat(x, y)				\
  x##y

#define a_concat(x, y)				\
  _a_concat(x, y)

#define a_max(x, y) ({				\
      __auto_type _x = x;			\
      __auto_type _y = y;			\
      _x > _y ? _x : _y;			\
    })						\

#define a_min(x, y) ({				\
      __auto_type _x = x;			\
      __auto_type _y = y;			\
      _x < _y ? _x : _y;			\
    })						\

#define a_unique(x)				\
  a_concat(x, __COUNTER__)

#endif
