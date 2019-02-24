#ifndef _COLOR_H_
#define _COLOR_H_

#include "common.h"

/* Color */
typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} rgb_t;

rgb_t *CreateColor(uint8_t red, uint8_t green, uint8_t blue);
rgb_t *CloneColor(rgb_t const *color);

void FreeColor(rgb_t *color);
static void FreeVoidColor(void *color) __unused;
static void FreeVoidColor(void *color) { FreeColor((rgb_t *)color); }

#endif /* _COLOR_H_ */
