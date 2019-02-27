/*
 * Mazart - RGB Color
 *  A generic 24-bit true color struct.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _COLOR_H_
#define _COLOR_H_

#include "common.h"

/* RGB Color */
typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  /* Maybe add an alpha channel later. */
} rgb_t;

/* Color constructors - Each allocates a new struct. */
rgb_t *CreateColor(uint8_t red, uint8_t green, uint8_t blue);
rgb_t *CloneColor(rgb_t const *color);
/* Color destructor */
void FreeColor(rgb_t *color);

/* Special free used for data structures that allow for user-specified
 * destructors (Ex. FreeGridDestroyCells()) */
static void FreeVoidColor(void *color) __unused;
static void FreeVoidColor(void *color) { FreeColor((rgb_t *)color); }

#endif /* _COLOR_H_ */
