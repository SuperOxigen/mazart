
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "deque.h"
#include "maze.h"
#include "maze_image.h"

static maze_flag_t const kPathDistanceProperty = 1;

// static size_t const kWidth = 190;
// static size_t const kHeight = 106;
static size_t const kWidth = 64;
static size_t const kHeight = 64;
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

static int64_t CountPathDistance(maze_t const *maze, point_t *path, size_t path_length)
{
  point_t pos;
  point_t poss[4];
  size_t height, width, i, j, n;
  int64_t dist, max_dist;
  maze_cell_t *cell, *next_cell;
  maze_cell_pair_t *conn;
  deque_t *conn_queue;
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
      EnqueueLast(conn_queue, conn);
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
      EnqueueLast(conn_queue, conn);
    }
  }
  FreeDeque(conn_queue);
  return max_dist;
}

bool_t CellColorGen(void *ctx, maze_cell_t const *cell, rgb_t *color)
{
  int64_t const *max_dist;
  int64_t dist;
  double frac;
  if (!ctx || !cell || !color) return false;
  max_dist = ctx;
  dist = GetMazeCellProperty(cell, kPathDistanceProperty);
  frac = ((double) dist) / ((double) (*max_dist + 1));
  *color = (rgb_t) {
    .red = 0,
    .green = (uint8_t) (256.0 * (1.0 - frac)),
    .blue = (uint8_t) (256.0 * frac)
  };
  return true;
}

bool_t ConnColorGen(void *ctx, maze_cell_t const *a, maze_cell_t const *b, rgb_t *color)
{
  int64_t const *max_dist;
  int64_t adist, bdist;
  double frac;
  if (!ctx || !a || !b || !color) return false;
  max_dist = ctx;
  adist = GetMazeCellProperty(a, kPathDistanceProperty);
  bdist = GetMazeCellProperty(b, kPathDistanceProperty);
  frac = ((double) (adist + bdist)) / ((double) (*max_dist * 2 + 1));
  *color = (rgb_t) {
    .red = 0,
    .green = (uint8_t) (256.0 * (1.0 - frac)),
    .blue = (uint8_t) (256.0 * frac)
  };
  return true;
}

int main(int argc __unused, char **argv __unused)
{
  point_t *path;
  size_t path_length;
  int64_t max_dist;
  maze_t *maze;
  maze_image_t *image;
  maze_image_config_t config;
  srand(5);
  printf("Creating Maze... ");
  maze = CreateMaze(kHeight, kWidth, &kStart, &kEnd);
  printf("Done\n");

  printf("Computing maze path... ");
  path = calloc(kWidth*kHeight+1, sizeof(point_t));
  path_length = ComputeMazePath(maze, &kStart, &kEnd, path, kWidth*kHeight+1);
  printf("Done\n");
  printf("Path found, length = %lu\n", path_length);

  printf("Finding max path distance... ");
  max_dist = CountPathDistance(maze, path, path_length);
  printf("Done\n");
  printf("Max distance from path is %ld\n", max_dist);

  printf("Converting maze to image... ");
  DefaultMazeImageConfig(&config);
  // config.border_width = 10;
  // config.cell_width = 7;
  // config.wall_width = 3;
  config.cell_color_gen = CellColorGen;
  config.cell_color_ctx = &max_dist;
  config.conn_color_gen = ConnColorGen;
  config.conn_color_ctx = &max_dist;
  image = CreateMazeImage(maze, &config);
  printf("Done\n");

  printf("Exporting unsolved maze... ");
  ExportImageToPNG(image, "maze.png");
  printf("Done\n");

  printf("Drawing solution on maze image... ");
  DrawPathOnMazeImage(image, path, path_length, &kRed);
  printf("Done\n");

  printf("Exporting solved maze... ");
  ExportImageToPNG(image, "maze.sol.png");
  printf("Done\n");

  free(path);
  FreeMazeImage(image);
  FreeMaze(maze);

  return 0;
}
