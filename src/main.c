
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "grid.h"
#include "maze.h"
#include "maze_image.h"

static size_t const kWidth = 190;
static size_t const kHeight = 106;
static point_t const kStart = {
  .row = 0,
  .col = kWidth - 1
};
static point_t const kEnd = {
  .row = kHeight - 1,
  .col = 0
};

static rgb_t const kRed = {
  .red = 255, 0
};

static bool_t cell_to_color(maze_cell_t const *cell, rgb_t *color)
{
  point_t pos;
  if (!cell) return false;
  GetMazeCellPosition(cell, &pos);
  *color = (rgb_t) {
    .red = pos.col % 256,
    .green = pos.row % 256,
    .blue = ((pos.col % 256) * (pos.row % 256)) % 256
  };
  return true;
}

int main(int argc __unused, char **argv __unused)
{
  point_t *path;
  size_t path_length;
  maze_t *maze;
  maze_image_t *image;
  maze_image_config_t config;
  srand(5);
  printf("Creating Maze\n");
  maze = CreateMaze(kHeight, kWidth, &kStart, &kEnd);
  printf("Computing maze path\n");
  path = calloc(kWidth*kHeight+1, sizeof(point_t));
  path_length = ComputeMazePath(maze, &kStart, &kEnd, path, kWidth*kHeight+1);
  printf("Path found in %lu steps\n", path_length);

  printf("Creating image\n");
  DefaultMazeImageConfig(&config);
  config.border_width = 10;
  config.cell_width = 10;
  config.wall_width = 0;
  config.cell_color_gen = cell_to_color;
  image = CreateMazeImage(maze, &config);
  ExportImageToPNG(image, "maze.png");
  DrawPathOnMazeImage(image, path, path_length, &kRed);
  ExportImageToPNG(image, "maze.sol.png");

  FreeMazeImage(image);
  FreeMaze(maze);

  return 0;
}
