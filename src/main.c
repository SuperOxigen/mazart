
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "grid.h"
#include "maze.h"
#include "maze_image.h"

static size_t const kWidth = 15;
static size_t const kHeight = 8;
static point_t const kStart = {
  .row = 0,
  .col = kWidth - 1
};
static point_t const kEnd = {
  .row = kHeight - 1,
  .col = 0
};

static maze_flag_t const kVisitedFlag = 3;

static void draw_maze(maze_t *maze, grid_t *image, maze_cell_t *cell)
{
  point_t poss[4];
  maze_cell_t *next;
  size_t n, i;
  point_t ipos, cpos;
  SetMazeCellFlag(cell, kVisitedFlag, true);
  GetMazeCellPosition(cell, &cpos);
  ipos.row = cpos.row * 2 + 1;
  ipos.col = cpos.col * 2 + 1;
  SetGridCell(image, &ipos, (void *)" ");

  n = GetMazeCellNeighbourPoints(cell, poss);
  for (i = 0; i < n; i++)
  {
    next = GetMazeCell(maze, &poss[i]);
    if (GetMazeCellFlag(next, kVisitedFlag)) continue;
    ipos.row = poss[i].row + cpos.row + 1;
    ipos.col = poss[i].col + cpos.col + 1;
    SetGridCell(image, &ipos, (void *)" ");
    draw_maze(maze, image, next);
  }
}

static void draw_path(grid_t *image, point_t *path, size_t path_length)
{
  size_t i;
  point_t ipos;

  ipos.row = path[0].row * 2 + 1;
  ipos.col = path[0].col * 2 + 1;
  SetGridCell(image, &ipos, (void *)"+");
  for (i = 1; i < path_length; i++)
  {
    ipos.row = path[i].row + path[i-1].row + 1;
    ipos.col = path[i].col + path[i-1].col + 1;
    SetGridCell(image, &ipos, (void *) ((path[i].row == path[i-1].row) ? "-" : "|"));

    ipos.row = path[i].row * 2 + 1;
    ipos.col = path[i].col * 2 + 1;
    SetGridCell(image, &ipos, (void *)"+");
  }
}

static void print_image(grid_t *image)
{
  size_t h, w;
  point_t pos;
  char const *c;
  h = GridHeight(image);
  w = GridWidth(image);
  for (pos.row = 0; pos.row < h; pos.row++)
  {
    for (pos.col = 0; pos.col < w; pos.col++)
    {
      c = GetGridCell(image, &pos);
      if (!c) putchar('@');
      else putchar(*c);
    }
    putchar('\n');
  }
}

static void fill_image(grid_t *image, char const *fill)
{
  size_t h, w;
  point_t pos;
  h = GridHeight(image);
  w = GridWidth(image);
  for (pos.row = 0; pos.row < h; pos.row++)
  {
    for (pos.col = 0; pos.col < w; pos.col++)
    {
      SetGridCell(image, &pos, (void*)fill);
    }
  }
}

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

  FreeMazeImage(image);
  FreeMaze(maze);


  return 0;
}
