/*
 * Mazart - Colorer
 *  A set of built-in coloring functions.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "colorer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "maze.h"

typedef enum {
  CLRR_PALETTE,
  CLRR_PALETTE_GRADIENT,
  CLRR_GRADIENT,
  CLRR_MAX
} colorer_type_t;

struct colorer_ctx_st {
  colorer_type_t type;
  maze_property_t property;
  int64_t min;
  int64_t max;
  rgb_t start_color;
  rgb_t end_color;
};

static rgb_t const kColorPalette[] = {
  #include "palette.data.c"
};
static size_t const kColorPaletteSize = sizeof(kColorPalette) / sizeof(rgb_t);

static colorer_ctx_t *CreateColorerContext(colorer_type_t type)
{
  colorer_ctx_t *ctx;
  ctx = calloc(1, sizeof(colorer_ctx_t));
  ctx->type = type;
  return ctx;
}

colorer_ctx_t *CreatePaletteColorerContext(maze_property_t property)
{
  colorer_ctx_t *ctx;
  ctx = CreateColorerContext(CLRR_PALETTE);
  ctx->property = property;
  return ctx;
}

colorer_ctx_t *CreatePaletteGradientColorerContext(maze_property_t property, int64_t min, int64_t max)
{
  colorer_ctx_t *ctx;
  if (min > max) return NULL;
  ctx = CreateColorerContext(CLRR_PALETTE_GRADIENT);
  ctx->property = property;
  ctx->max = max;
  ctx->min = min;
  return ctx;
}

colorer_ctx_t *CreateGradientColorContext(
  rgb_t const *start_color, rgb_t const *end_color,
  maze_property_t property, int64_t min, int64_t max)
{
  colorer_ctx_t *ctx;
  if (min > max || !start_color || !end_color) return NULL;
  ctx = CreateColorerContext(CLRR_GRADIENT);
  ctx->property = property;
  ctx->max = max;
  ctx->min = min;
  ctx->start_color = *start_color;
  ctx->end_color = *end_color;
  return ctx;
}

void FreeColorerContext(colorer_ctx_t *ctx)
{
  if (!ctx) return;
  memset(ctx, 0, sizeof(colorer_ctx_t));
  free(ctx);
}

bool_t ApplyColorerToMazeImageConfig(
  maze_image_config_t *config, colorer_ctx_t *ctx)
{
  if (!config || !ctx) return false;
  config->cell_color_ctx = ctx;
  config->cell_color_gen = CellColorer;
  config->conn_color_ctx = ctx;
  config->conn_color_gen = ConnColorer;
  return true;
}

bool_t CellColorer(void *vctx, maze_cell_t const *cell, rgb_t *color)
{
  colorer_ctx_t *ctx;
  int64_t prop_value;
  if (!vctx || !cell || !color) return false;
  ctx = vctx;
  prop_value = GetMazeCellProperty(cell, ctx->property);
  if (ctx->type == CLRR_PALETTE)
  {
    if (prop_value < 0) return false;
    *color = kColorPalette[prop_value & kColorPaletteSize];
    return true;
  }
  if (ctx->type == CLRR_PALETTE_GRADIENT)
  {
    double frac;
    if (prop_value > ctx->max) frac = ((double) (ctx->max - ctx->min)) / ((double) (ctx->max - ctx->min + 1));
    else if (prop_value < ctx->min) frac = 0.0;
    else frac = ((double) (prop_value - ctx->min)) / ((double) (ctx->max - ctx->min + 1));
    *color = kColorPalette[(size_t) (((double) kColorPaletteSize) * frac)];
    return true;
  }
  if (ctx->type == CLRR_GRADIENT)
  {
    double frac;
    if (prop_value > ctx->max) frac = ((double) (ctx->max - ctx->min)) / ((double) (ctx->max - ctx->min + 1));
    else if (prop_value < ctx->min) frac = 0.0;
    else frac = ((double) (prop_value - ctx->min)) / ((double) (ctx->max - ctx->min + 1));
    color->red = (uint8_t) (((double) ctx->start_color.red) * frac + ((double) ctx->end_color.red) * (1.0 - frac));
    color->green = (uint8_t) (((double) ctx->start_color.green) * frac + ((double) ctx->end_color.green) * (1.0 - frac));
    color->blue = (uint8_t) (((double) ctx->start_color.blue) * frac + ((double) ctx->end_color.blue) * (1.0 - frac));
    return true;
  }
  return false;
}

bool_t ConnColorer(void *vctx, maze_cell_t const *a, maze_cell_t const *b, rgb_t *color)
{
  rgb_t a_color, b_color;
  double av, bv;
  if (!vctx || !a || !b || !color) return false;
  if (!CellColorer(vctx, a, &a_color)) return false;
  if (!CellColorer(vctx, b, &b_color)) return false;
  av = (double) a_color.red;
  bv = (double) b_color.red;
  color->red = (uint8_t) sqrt(((av * av) + (bv * bv)) / 2.0);
  av = (double) a_color.green;
  bv = (double) b_color.green;
  color->green = (uint8_t) sqrt(((av * av) + (bv * bv)) / 2.0);
  av = (double) a_color.blue;
  bv = (double) b_color.blue;
  color->blue = (uint8_t) sqrt(((av * av) + (bv * bv)) / 2.0);
  return true;
}
