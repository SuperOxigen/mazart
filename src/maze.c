
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "grid.h"
#include "maze.h"

struct maze_st {
  grid_t *grid;
  /* Start and finish */
  point_t start;
  point_t end;
};

static void DrawMaze(maze_t *maze);
static void ClearMazeConnections(maze_t *maze);
static void ClearMazeVisitedFlags(maze_t *maze);

struct maze_cell_st {
  point_t pos;
  /* Neighbours */
  maze_cell_t *up;
  maze_cell_t *down;
  maze_cell_t *left;
  maze_cell_t *right;
  /* Flags */
  bool_t flags[MAX_MAZE_FLAG];
  /* Properties */
  int64_t properties[MAX_MAZE_PROPERTY];
  /* Private Flags*/
  bool_t visited;
};

#define visit(c) (c)->visited = true

static maze_cell_t *CreateMazeCell(point_t const *pos);
static void FreeMazeCell(maze_cell_t *cell);
static void FreeVoidMazeCell(void *cell) { FreeMazeCell((maze_cell_t*) cell); }
static void ConnectMazeCells(maze_cell_t *a, maze_cell_t *b);

/* - - Maze - - */

maze_t *CreateMaze(size_t height, size_t width, point_t const *start, point_t const *end)
{
  maze_t *maze;
  point_t p;
  if (!start || !end) return NULL;
  if (height == 0 || width == 0) return NULL;
  if (width <= start->col || width <= end->col) return NULL;
  if (height <= start->row || height <= end->row) return NULL;
  maze = (maze_t*)calloc(1, sizeof(maze_t));
  maze->grid = CreateGrid(height, width);
  assign_point(start, &maze->start);
  assign_point(end, &maze->end);
  for (p.row = 0; p.row < height; p.row++)
  {
    for (p.col = 0; p.col < width; p.col++)
    {
      SetGridCell(maze->grid, &p, CreateMazeCell(&p));
    }
  }
  DrawMaze(maze);
  return maze;
}

void FreeMaze(maze_t *maze)
{
  if (!maze) return;
  ClearGridDestroyCell(maze->grid, FreeVoidMazeCell);
  FreeGrid(maze->grid);
  memset(maze, 0, sizeof(maze_t));
  free(maze);
}

void ReDrawMaze(maze_t *maze, point_t const *start, point_t const *end)
{
  if (!maze) return;
  ClearMazeConnections(maze);
  if (start) assign_point(start, &maze->start);
  if (end) assign_point(end, &maze->end);
  DrawMaze(maze);
}

size_t MazeWidth(maze_t const *maze)
{
  if (!maze) return 0;
  return GridWidth(maze->grid);
}

size_t MazeHeight(maze_t const *maze)
{
  if (!maze) return 0;
  return GridHeight(maze->grid);
}

void MazeStart(maze_t const *maze, point_t *pos)
{
  if (!maze || !pos) return;
  assign_point(&maze->start, pos);
}

void MazeEnd(maze_t const *maze, point_t *pos)
{
  if (!maze || !pos) return;
  assign_point(&maze->end, pos);
}

maze_cell_t *GetMazeCell(maze_t const *maze, point_t const *pos)
{
  if (!maze || !pos) return NULL;
  return (maze_cell_t*)GetGridCell(maze->grid, pos);
}

maze_cell_t *GetMazeStartCell(maze_t const *maze)
{
  if (!maze) return NULL;
  return GetMazeCell(maze, &maze->start);
}

maze_cell_t *GetMazeEndCell(maze_t const *maze)
{
  if (!maze) return NULL;
  return GetMazeCell(maze, &maze->end);
}

/* - - Compute Path - - */
static size_t CrawlMazePath(
  maze_t const *maze,
  maze_cell_t *current, maze_cell_t *dest,
  point_t *path, size_t path_idx, size_t max_path)
{
  point_t poss[4];
  size_t n, i, res;
  maze_cell_t *next;
  if (path_idx >= max_path) return 0;
  visit(current);
  assign_point(&current->pos, &path[path_idx]);
  if (same_point(&current->pos, &dest->pos))
  {
    return path_idx + 1;
  }
  n = GetMazeCellNeighbourPoints(current, poss);
  for (i = 0; i < n; i++)
  {
    next = GetMazeCell(maze, &poss[i]);
    if (next->visited) continue;
    res = CrawlMazePath(
      maze,
      next, dest,
      path, path_idx + 1, max_path);
    if (res > 0) return res;
  }
  return 0;
}

size_t ComputeMazePath(
  maze_t const *maze, point_t const *a, point_t const *b,
  point_t *path, size_t max_path)
{
  if (!maze || !a || !b || !path) return 0;
  ClearMazeVisitedFlags((maze_t*)maze);
  return CrawlMazePath(
    maze,
    GetMazeCell(maze, a), GetMazeCell(maze, b),
    path, 0, max_path);
}

static void RemovePosition(point_t *poss, size_t idx, size_t n)
{
  size_t i;
  for (i = idx + 1; i < n; i++)
  {
    assign_point(&poss[i], &poss[i-1]);
  }
}

static void RandomizePositions(point_t *poss, size_t n)
{
  size_t i, j;
  for (i = 0; i < n; i++)
  {
    j = rand() % n;
    if (i == j) continue;
    swap_points(&poss[i], &poss[j]);
  }
}

static void CrawlMazeDrawing(maze_t *maze, maze_cell_t *current)
{
  point_t poss[4];
  size_t n, i;
  maze_cell_t *next;
  visit(current);
  /* Create a list of potential neighbours. */
  for (i = 0; i < 4; i++) assign_point(&current->pos, &poss[i]);
  n = 2;
  poss[0].col++;
  poss[1].row++;
  if (poss[n].col > 0) poss[n++].col--;
  if (poss[n].row > 0) poss[n++].row--;
  /* Remove non-existent or visited neighbours */
  for (i = 0; i < n; i++)
  {
    next = GetMazeCell(maze, &poss[i]);
    if (next && !next->visited) continue;
    RemovePosition(poss, i--, n--);
  }
  if (n == 0) return;
  /* Randomize remaining */
  RandomizePositions(poss, n);
  /* Go through each */
  for (i = 0; i < n; i++)
  {
    next = GetMazeCell(maze, &poss[i]);
    /* Check that while visiting one, that it hasn't vistsed the next */
    if (next->visited) continue;
    ConnectMazeCells(current, next);
    CrawlMazeDrawing(maze, next);
  }
}

static void DrawMaze(maze_t *maze)
{
  ClearMazeVisitedFlags(maze);
  CrawlMazeDrawing(maze, GetMazeCell(maze, &maze->start));
}

static void ClearMazeConnections(maze_t *maze)
{
  point_t pos;
  size_t height, width;
  maze_cell_t *cell;
  if (!maze) return;
  height = MazeHeight(maze);
  width = MazeWidth(maze);
  for (pos.row = 0; pos.row < height; pos.row++)
  {
    for (pos.col = 0; pos.col < width; pos.col++)
    {
      cell = GetGridCell(maze->grid, &pos);
      cell->up = NULL;
      cell->down = NULL;
      cell->left = NULL;
      cell->right = NULL;
    }
  }
}

static void ClearMazeVisitedFlags(maze_t *maze)
{
  point_t pos;
  size_t height, width;
  maze_cell_t *cell;
  if (!maze) return;
  height = MazeHeight(maze);
  width = MazeWidth(maze);
  for (pos.row = 0; pos.row < height; pos.row++)
  {
    for (pos.col = 0; pos.col < width; pos.col++)
    {
      cell = GetGridCell(maze->grid, &pos);
      cell->visited = false;
    }
  }
}

/* - - Maze Cell - - */
static maze_cell_t *CreateMazeCell(point_t const *pos)
{
  maze_cell_t *cell;
  if (!pos) return NULL;
  cell = (maze_cell_t*)calloc(1, sizeof(maze_cell_t));
  assign_point(pos, &cell->pos);
  return cell;
}

static void FreeMazeCell(maze_cell_t *cell)
{
  if (!cell) return;
  memset(cell, 0, sizeof(maze_cell_t));
  free(cell);
}

void GetMazeCellPosition(maze_cell_t const *cell, point_t *pos)
{
  if (!cell || !pos) return;
  assign_point(&cell->pos, pos);
}

bool_t GetMazeCellFlag(maze_cell_t const *cell, maze_flag_t flag)
{
  if (!cell || flag >= MAX_MAZE_FLAG) return false;
  return cell->flags[flag];
}

void SetMazeCellFlag(maze_cell_t *cell, maze_flag_t flag, bool_t value)
{
  if (!cell || flag >= MAX_MAZE_FLAG) return;
  cell->flags[flag] = value;
}

int64_t GetMazeCellProperty(maze_cell_t const *cell, maze_property_t property)
{
  if (!cell || property >= MAX_MAZE_PROPERTY) return 0;
  return cell->properties[property];
}

void SetMazeCellProperty(maze_cell_t *cell, maze_property_t property, int64_t value)
{
  if (!cell || property >= MAX_MAZE_PROPERTY) return;
  cell->properties[property] = value;
}

void IncMazeCellProperty(maze_cell_t *cell, maze_property_t property)
{
  if (!cell || property >= MAX_MAZE_PROPERTY) return;
  cell->properties[property]++;
}

void DecMazeCellProperty(maze_cell_t *cell, maze_property_t property)
{
  if (!cell || property >= MAX_MAZE_PROPERTY) return;
  cell->properties[property]--;
}

size_t GetMazeCellNeighbourPoints(maze_cell_t const *cell, point_t *neighbours)
{
  size_t i;
  if (!cell || !neighbours) return 0;
  i = 0;
  if (cell->up) assign_point(&cell->up->pos, &neighbours[i++]);
  if (cell->down) assign_point(&cell->down->pos, &neighbours[i++]);
  if (cell->left) assign_point(&cell->left->pos, &neighbours[i++]);
  if (cell->right) assign_point(&cell->right->pos, &neighbours[i++]);
  return i;
}

static void ConnectMazeCells(maze_cell_t *a, maze_cell_t *b)
{
  if (!a || !b) return;
  if (same_point(&a->pos, &b->pos)) return;
  if (a->pos.col == b->pos.col)
  {
    if ((a->pos.row + 1) == b->pos.row)
    {
      a->up = b;
      b->down = a;
    }
    else if (a->pos.row == (b->pos.row + 1))
    {
      a->down = b;
      b->up = a;
    }
  }
  else if (a->pos.row == b->pos.row)
  {
    if ((a->pos.col + 1) == b->pos.col)
    {
      a->right = b;
      b->left = a;
    }
    else if (a->pos.col == (b->pos.col + 1))
    {
      a->left = b;
      b->right = a;
    }
  }
  return;
}
