
#include <stdlib.h>
#include <string.h>

#include "color.h"

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
