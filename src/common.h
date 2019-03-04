/*
 * Mazart - Common Include
 *  A bunch of commonly included headers, some type definitions,
 *  and a few compiler attributes.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h> /* Boolean types */
#include <stddef.h>  /* size_t and NULL */
#include <stdint.h>  /* Integer types */

typedef bool bool_t;
#undef bool  /* Forces the use of bool_t only, */

typedef char const *kstring_t;

/* - - Point Type - - */

typedef struct {
  size_t row;
  size_t col;
} point_t;

/* Tests equality of two points. */
static inline bool_t PointsEqual(point_t const *a, point_t const *b)
{
  return (a->row == b->row) && (a->col == b->col);
}

static inline void SwapPoints(point_t *a, point_t *b)
{
  size_t t;
  t = a->col; a->col = b->col; b->col = t;
  t = a->row; a->row = b->row; b->row = t;
}

/* - - Compiler Attributes - - */

/* Attributes are only available with GCC. */
#ifdef __GNUC__
/* Surpresses warnings when variables or functions are unused. */
#define __unused __attribute__((unused))
#else /* no __GNUC__ */
#define __unused
#endif

#endif /* _COMMON_H_ */
