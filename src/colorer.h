/*
 * Mazart - Colorer
 *  A set of built-in coloring functions.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _COLORER_H_
#define _COLORER_H_

#include "common.h"
#include "maze_image.h"

/*
 * Colorer Context Struct
 *  Contains all the contextual information needed to color a maze using
 *  the supported Maze Image API.
 *  When using this context, you MUST use CellColorer() and ConnColorer()
 *  as the color generators of the Maze Image config.
 */
typedef struct colorer_ctx_st colorer_ctx_t;

/* - - Colorer Context API - - */

colorer_ctx_t *CreatePaletteColorerContext(maze_property_t property);
colorer_ctx_t *CreatePaletteGradientColorerContext(maze_property_t property, int64_t min, int64_t max);
colorer_ctx_t *CreateGradientColorContext(
  rgb_t const *start_color, rgb_t const *end_color,
  maze_property_t property, int64_t min, int64_t max);

void FreeColorerContext(colorer_ctx_t *ctx);

bool_t ApplyColorerToMazeImageConfig(
  maze_image_config_t *config, colorer_ctx_t *ctx);

/* - - Coloror Generators - - */

bool_t CellColorer(void *ctx, maze_cell_t const *cell, rgb_t *color);
bool_t ConnColorer(void *ctx, maze_cell_t const *a, maze_cell_t const *b, rgb_t *color);

#endif /* _COLORER_H_ */
