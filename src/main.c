
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "grid.h"
#include "maze.h"
#include "maze_image.h"

static size_t const kWidth = 80;
static size_t const kHeight = 80;
static point_t const kStart = {
  .row = 0,
  .col = kWidth - 1
};
static point_t const kEnd = {
  .row = kHeight - 1,
  .col = 0
};

int main(int argc __unused, char **argv __unused)
{
  point_t *path;
  size_t path_length;
  maze_t *maze;
  maze_image_t *image;
  srand(5);
  printf("Creating Maze\n");
  maze = CreateMaze(kHeight, kWidth, &kStart, &kEnd);
  printf("Computing maze path\n");
  path = calloc(kWidth*kHeight+1, sizeof(point_t));
  path_length = ComputeMazePath(maze, &kStart, &kEnd, path, kWidth*kHeight+1);
  printf("Path found in %lu steps\n", path_length);

  printf("Creating image\n");
  image = CreateMazeImage(maze, NULL);
  ExportImageToPNG(image, "sample.png");

  FreeMazeImage(image);
  FreeMaze(maze);


  return 0;
}
