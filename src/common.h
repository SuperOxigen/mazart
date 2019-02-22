#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool bool_t;

typedef struct {
  size_t row;
  size_t col;
} point_t;

static inline void assign_point(point_t const *src, point_t *dest)
{
  dest->row = src->row;
  dest->col = src->col;
}

static inline bool_t same_point(point_t const *a, point_t const *b)
{
  return (a->row == b->row) && (a->col == b->col);
}

static inline void swap_points(point_t *a, point_t *b)
{
  size_t t;
  t = a->col;
  a->col = b->col;
  b->col = t;
  t = a->row;
  a->row = b->row;
  b->row = t;
}

#ifdef __GNUC__
#define __unused __attribute__((unused))
#else /* no __GNUC__ */
#define __unused
#endif

#endif /* _COMMON_H_ */
