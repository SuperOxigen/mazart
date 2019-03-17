/*
 * Mazart - Main
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "colorer.h"
#include "common.h"
#include "config.h"
#include "deque.h"
#include "maze.h"
#include "maze_image.h"

static maze_property_t const kPathDistanceProperty = 1;
static maze_property_t const kStartDistanceProperty = 2;
static maze_property_t const kEndDistanceProperty = 3;

static rgb_t const kPresetAStartColor = {.red = 255, .green = 127, .blue = 0};
static rgb_t const kPresetAEndColor = {.red = 127, .green = 0, .blue = 127};

typedef struct {
  int64_t path_max;
  int64_t start_max;
  int64_t end_max;
} mazart_maxes_t;

static int64_t CountDistanceFromPath(maze_t const *maze, point_t *path, size_t path_length)
{
  point_t pos;
  point_t poss[4];
  size_t height, width, i, j, n;
  int64_t dist, max_dist;
  maze_cell_t *cell, *next_cell;
  maze_cell_pair_t *conn;
  deque_t *conn_queue;
  if (!maze || !path || path_length == 0) return -1;
  height = MazeHeight(maze);
  width = MazeWidth(maze);
  /* Clear path distance data */
  for (pos.row = 0; pos.row < height; pos.row++)
  for (pos.col = 0; pos.col < width; pos.col++)
  {
    cell = GetMazeCell(maze, &pos);
    if (!cell) continue;
    SetMazeCellProperty(cell, kPathDistanceProperty, 0);
  }
  /* Initalize all the cells on the path. */
  for (i = 0; i < path_length; i++)
  {
    cell = GetMazeCell(maze, &path[i]);
    if (!cell) continue;
    SetMazeCellProperty(cell, kPathDistanceProperty, 1);
  }
  /* Queue all the neighbours of the path for processing. */
  conn_queue = CreateDeque();
  for (i = 0; i < path_length; i++)
  {
    cell = GetMazeCell(maze, &path[i]);
    if (!cell) continue;
    n = GetMazeCellNeighbourPoints(cell, poss);
    for (j = 0; j < n; j++)
    {
      next_cell = GetMazeCell(maze, &poss[j]);
      if (!next_cell) continue;
      /* Skip nodes on the path */
      if (GetMazeCellProperty(next_cell, kPathDistanceProperty) == 1) continue;
      conn = CreateMazeCellPair(cell, next_cell);
      PushDequeLast(conn_queue, conn);
    }
  }
  max_dist = 1;
  while (DequeSize(conn_queue) > 0)
  {
    conn = PopDequeFirst(conn_queue);
    cell = conn->src;
    next_cell = conn->dest;
    FreeMazeCellPair(conn);
    /* Set distance */
    dist = GetMazeCellProperty(cell, kPathDistanceProperty) + 1;
    if (dist > max_dist)
    {
      max_dist = dist;
    }
    SetMazeCellProperty(next_cell, kPathDistanceProperty, dist);
    /* Queue all neightbours */
    cell = next_cell;
    n = GetMazeCellNeighbourPoints(cell, poss);
    for (i = 0; i < n; i++)
    {
      next_cell = GetMazeCell(maze, &poss[i]);
      if (!next_cell) continue;
      /* Skip nodes on the path */
      if (GetMazeCellProperty(next_cell, kPathDistanceProperty) > 0) continue;
      conn = CreateMazeCellPair(cell, next_cell);
      PushDequeLast(conn_queue, conn);
    }
  }
  FreeDeque(conn_queue);
  return max_dist;
}

static int64_t CountDistanceFromSource(maze_t const *maze, point_t const *source_pos, maze_property_t property)
{
  point_t pos;
  point_t poss[4];
  size_t height, width, i, n;
  int64_t dist, max_dist;
  maze_cell_t *cell, *next_cell;
  maze_cell_pair_t *conn;
  deque_t *conn_queue;
  if (!maze || !source_pos) return -1;
  height = MazeHeight(maze);
  width = MazeWidth(maze);
  /* Clear cell distance data */
  for (pos.row = 0; pos.row < height; pos.row++)
  for (pos.col = 0; pos.col < width; pos.col++)
  {
    cell = GetMazeCell(maze, &pos);
    if (!cell) continue;
    SetMazeCellProperty(cell, property, 0);
  }
  /* Initalize First */
  cell = GetMazeCell(maze, source_pos);
  if (!cell) return -1;
  SetMazeCellProperty(cell, property, 1);
  /* Queue first set of neighbours. */
  conn_queue = CreateDeque();
  n = GetMazeCellNeighbourPoints(cell, poss);
  for (i = 0; i < n; i++)
  {
    next_cell = GetMazeCell(maze, &poss[i]);
    if (!next_cell) return -1;
    conn = CreateMazeCellPair(cell, next_cell);
    PushDequeLast(conn_queue, conn);
  }
  max_dist = 1;
  while (DequeSize(conn_queue) > 0)
  {
    conn = PopDequeFirst(conn_queue);
    cell = conn->src;
    next_cell = conn->dest;
    FreeMazeCellPair(conn);
    /* Set distance */
    dist = GetMazeCellProperty(cell, property) + 1;
    if (dist > max_dist)
    {
      max_dist = dist;
    }
    SetMazeCellProperty(next_cell, property, dist);
    /* Queue all neightbours */
    cell = next_cell;
    n = GetMazeCellNeighbourPoints(cell, poss);
    for (i = 0; i < n; i++)
    {
      next_cell = GetMazeCell(maze, &poss[i]);
      if (!next_cell) continue;
      /* Skip nodes on the path */
      if (GetMazeCellProperty(next_cell, property) > 0) continue;
      conn = CreateMazeCellPair(cell, next_cell);
      PushDequeLast(conn_queue, conn);
    }
  }
  FreeDeque(conn_queue);
  return max_dist;
}

static colorer_ctx_t *CreateColorerContextFromConfig(mazart_config_t const *config, mazart_maxes_t const *maxes)
{
  colorer_ctx_t *ctx;
  maze_property_t property;
  int64_t max;
  if (!config || !maxes) return NULL;
  switch (config->cell_color_metric)
  {
    case CLR_MTRC_PATH_DIST:
      property = kPathDistanceProperty;
      max = maxes->path_max;
      break;
    case CLR_MTRC_START_DIST:
      property = kStartDistanceProperty;
      max = maxes->start_max;
      break;
    case CLR_MTRC_END_DIST:
      property = kEndDistanceProperty;
      max = maxes->end_max;
      break;
    case CLR_MTRC_OTHER_DIST:
      fprintf(stderr, "Warning: Cell \"other\" metric is not supported\n");
      return NULL;
    default:
      return NULL;
  }
  switch (config->cell_color_mode)
  {
    case CLR_MODE_PALETTE:
      ctx = CreatePaletteGradientColorerContext(property, 0, max);
      if (config->cell_color_palette_offset > 0)
        SetPaletteColorerOffset(ctx, config->cell_color_palette_offset);
      if (config->cell_color_palette_reverse)
        SetPaletteColorerReverse(ctx, true);
      return ctx;
    case CLR_MODE_PRESET_A:
      return CreateGradientColorContext(&kPresetAStartColor, &kPresetAEndColor, property, 0, max);
    case CLR_MODE_NONE:
    default:
      return NULL;
  }
}

static void ConvertConfigToMazeImageConfig(mazart_config_t const *config, maze_image_config_t *img_config, mazart_maxes_t const *maxes)
{
  if (!config || !img_config || !maxes) return;
  DefaultMazeImageConfig(img_config);
  img_config->cell_width = config->cell_width;
  img_config->wall_width = config->wall_width;
  img_config->border_width = config->border_width;
  if (config->cell_color_mode != CLR_MODE_NONE)
  {
    colorer_ctx_t *ctx;
    ctx = CreateColorerContextFromConfig(config, maxes);
    if (!ctx)
    {
      fprintf(stderr,
        "Warning: Problem occurred while creating colorer context\n");
    }
    ApplyColorerToMazeImageConfig(img_config, ctx);
  }
  else
  {
    MazartColorToColor(config->cell_color, &img_config->default_cell_color);
    MazartColorToColor(config->conn_color, &img_config->default_conn_color);
  }
  MazartColorToColor(config->wall_color, &img_config->wall_color);
  MazartColorToColor(config->border_color, &img_config->border_color);
  MazartColorToColor(config->path_color, &img_config->default_path_color);
}

static void ConvertConfigToMazeStartEnd(mazart_config_t const *config, point_t *start, point_t *end)
{
  if (start)
  {
    start->col = config->maze_width - 1;
    start->row = 0;
  }
  if (end)
  {
    end->col = 0;
    end->row = config->maze_height - 1;
  }
}

static maze_t *CreateMazeFromConfig(mazart_config_t const *config)
{
  point_t start, end;
  if (!config) return NULL;
  ConvertConfigToMazeStartEnd(config, &start, &end);
  return CreateMaze(config->maze_height, config->maze_width, &start, &end);
}

int main(int argc, char **argv)
{
  point_t start, end;
  point_t *path = NULL;
  size_t path_length;
  mazart_maxes_t maxes;
  maze_t *maze;
  maze_image_t *image;
  maze_image_config_t img_config;
  mazart_config_t config;

  if (!ParseMazartParameters((void*) argv, argc, &config))
  {
    return EXIT_FAILURE;
  }
  printf("Generating %lu x %lu maze, saving to %s\n",
    config.maze_width, config.maze_height, config.output_file);
  if (config.debug_mode) PrintMazartConfit(&config);

  if (config.debug_mode) printf("Applying seed %lu\n", config.seed);
  srand(config.seed);

  if (config.debug_mode) printf("Creating Maze...\n");
  maze = CreateMazeFromConfig(&config);

  if (config.debug_mode) printf("Computing maze path...\n");
  ConvertConfigToMazeStartEnd(&config, &start, &end);
  path = calloc(config.maze_width * config.maze_height + 1, sizeof(point_t));
  path_length = ComputeMazePath(maze,
    &start, &end,
    path, config.maze_width * config.maze_height + 1);
  if (config.debug_mode) printf("Path found, length = %lu\n", path_length);

  if (config.debug_mode) printf("Finding max path distance...\n");
  maxes.path_max = CountDistanceFromPath(maze, path, path_length);
  if (config.debug_mode) printf("Max distance from path is %ld\n", maxes.path_max);

  if (config.debug_mode) printf("Finding max distance from start...\n");
  maxes.start_max = CountDistanceFromSource(maze, &start, kStartDistanceProperty);
  if (config.debug_mode) printf("Max distance from start is %ld\n", maxes.start_max);

  if (config.debug_mode) printf("Finding max distance from end...\n");
  maxes.end_max = CountDistanceFromSource(maze, &end, kEndDistanceProperty);
  if (config.debug_mode) printf("Max distance from end is %ld\n", maxes.end_max);

  if (config.debug_mode) printf("Converting maze to image...\n");
  ConvertConfigToMazeImageConfig(&config, &img_config, &maxes);
  image = CreateMazeImage(maze, &img_config);

  if (config.draw_path)
  {
    if (config.debug_mode) printf("Drawing solution path...\n");
    DrawPathOnMazeImage(image, path, path_length, NULL);
  }

  if (config.debug_mode) printf("Exporting maze to %s...\n", config.output_file);
  ExportMazeImageToPNG(image, config.output_file);

  free(path);
  FreeColorerContext(img_config.cell_color_ctx);
  FreeMazeImage(image);
  FreeMaze(maze);

  return 0;
}
