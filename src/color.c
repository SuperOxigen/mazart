/*
 * Mazart - RGB Color
 *  A generic 24-bit true color struct.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "color.h"

#include <stdlib.h>
#include <string.h>

rgb_t *CreateColor(uint8_t red, uint8_t green, uint8_t blue)
{
  rgb_t *color;
  color = (rgb_t *)calloc(1, sizeof(rgb_t));
  color->red = red;
  color->green = green;
  color->blue = blue;
  return color;
}

rgb_t *CloneColor(rgb_t const *color)
{
  if (!color) return NULL;
  return CreateColor(color->red, color->green, color->blue);
}

void FreeColor(rgb_t *color)
{
  if (!color) return;
  free(color);
}
