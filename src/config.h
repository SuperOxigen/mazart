/*
 * Mazart - Program Configuration
 *  Parses commandline arguments and produces a set of configuration
 *  options.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"
#include "color.h"

typedef enum {
  CLR_NONE,
  /* Grey scale. */
  CLR_WHITE,
  CLR_LIGHT_GREY,
  CLR_GREY,
  CLR_BLACK,
  /* Colors. */
  CLR_BLUE,
  CLR_TEAL,
  CLR_GREEN,
  CLR_YELLOW,
  CLR_ORANGE,
  CLR_RED,
  CLR_PURPLE,
  /* Other. */
  CLR_OTHER
} mazart_color_t;

typedef enum {
  CLR_MTRC_NONE,
  CLR_MTRC_PATH_DIST,
  CLR_MTRC_START_DIST,
  CLR_MTRC_END_DIST,
  CLR_MTRC_OTHER_DIST
} mazart_color_metric_t;

typedef enum {
  CLR_MODE_NONE,
  CLR_MODE_PALETTE,
  CLR_MODE_PRESET_A
} mazart_color_mode_t;

typedef enum {
  CLR_MTHD_NONE,
  CLR_MTHD_FIXED,
  CLR_MTHD_NEAREST,
  CLR_MTHD_AVERAGE
} mazart_color_method_t;

typedef struct {
  /* Maze parameters. */
  size_t maze_width;
  size_t maze_height;
  /* Randomizer config. */
  size_t seed;
  /* Image settings. */
  size_t cell_width;
  size_t wall_width;
  size_t border_width;
  /* Cell color settings. */
  mazart_color_t cell_color;
  mazart_color_metric_t cell_color_metric;
  mazart_color_mode_t cell_color_mode;
  /* Conn color settings. */
  mazart_color_t conn_color;
  mazart_color_method_t conn_color_method;
  /* Wall color. */
  mazart_color_t wall_color;
  /* Border color. */
  mazart_color_t border_color;
  /* Output file. */
  char const *output_file;
} mazart_config_t;

void MazartDefaultParameters(mazart_config_t *config);
void PrintMazartConfit(mazart_config_t *config);

bool_t ParseMazartParameters(char const * const *args, size_t arg_count, mazart_config_t *config);

bool_t MazartColorToColor(mazart_color_t ma_color, rgb_t *color);

#endif /* _CONFIG_H_ */
