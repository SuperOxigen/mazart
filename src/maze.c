/*
 * Mazart - Maze
 *  Module provides a Maze generating object.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "maze.h"

#include <stdlib.h>
#include <string.h>

#include "grid.h"
#include "priority.h"

/* - - Maze Structure - - */

struct maze_st {
  grid_t *grid;
  point_t start;
  point_t end;
};

/* - - Maze Internal API Prototypes - - */

/* Creates the connections between cells. */
static void DrawMaze(maze_t *maze);
/* Removes all connections between cells. */
static void ClearMazeConnections(maze_t *maze);
/* Clears all Maze Cell's `visited` flag. */
static void ClearMazeVisitedFlags(maze_t *maze);

/* - - Maze Cell Structure - - */

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
  /* Private Flags */
  bool_t visited; /* Used for several traveral algorithms. */
};

/* - - Maze Cell Internal API Prototypes - - */

/* Maze Cell constructor. */
static maze_cell_t *CreateMazeCell(point_t const *pos);
/* Maze Cell destructor. */
static void FreeMazeCell(maze_cell_t *cell);
/* Special destructor signature used for ClearGridDestroyCells(). */
static void FreeVoidMazeCell(void *cell) { FreeMazeCell((maze_cell_t*) cell); }

#define visit(c) (c)->visited = true
/* Creates a bi-directional connection between two given cells. */
static void ConnectMazeCells(maze_cell_t *a, maze_cell_t *b);

/* - - Maze API - - */

maze_t *CreateMaze(size_t height, size_t width, point_t const *start, point_t const *end)
{
  maze_t *maze;
  point_t p;
  if (!start || !end) return NULL;
  if (height == 0 || width == 0) return NULL;
  /* Enforce start end bounds. */
  if (width <= start->col || width <= end->col) return NULL;
  if (height <= start->row || height <= end->row) return NULL;
  /* Create Maze struct. */
  maze = (maze_t*)calloc(1, sizeof(maze_t));
  /* Create Grid for storing Maze Cells. */
  maze->grid = CreateGrid(height, width);
  maze->start = *start;
  maze->end = *end;
  /* Creates a new Maze Cell for every point.  */
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
  ClearGridDestroyCells(maze->grid, FreeVoidMazeCell);
  FreeGrid(maze->grid);
  memset(maze, 0, sizeof(maze_t));
  free(maze);
}

void ReDrawMaze(maze_t *maze, point_t const *start, point_t const *end)
{
  if (!maze) return;
  ClearMazeConnections(maze);
  if (start) maze->start = *start;
  if (end) maze->end = *end;
  DrawMaze(maze);
}

/* - Maze getters. - */

size_t MazeHeight(maze_t const *maze)
{
  if (!maze) return 0;
  return GridHeight(maze->grid);
}

size_t MazeWidth(maze_t const *maze)
{
  if (!maze) return 0;
  return GridWidth(maze->grid);
}

void MazeStart(maze_t const *maze, point_t *pos)
{
  if (!maze || !pos) return;
  *pos = maze->start;
}

void MazeEnd(maze_t const *maze, point_t *pos)
{
  if (!maze || !pos) return;
  *pos = maze->end;
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

size_t ComputeMazePath(
  maze_t const *maze, point_t const *src, point_t const *dest,
  point_t *path, size_t max_path)
{
  size_t pidx;
  if (!maze || !src || !dest || !path || max_path == 0) return 0;
  if (!GetMazeCell(maze, src) || !GetMazeCell(maze, dest)) return 0;
  ClearMazeVisitedFlags((maze_t*)maze);
  pidx = 0;
  path[pidx] = *src;
  visit(GetMazeCell(maze, src));
  while (!PointsEqual(&path[pidx], dest))
  {
    point_t poss[4];
    size_t n, i;
    n = GetMazeCellNeighbourPoints(GetMazeCell(maze, &path[pidx]), poss);
    for (i = 0; i < n; i++)
    {
      if (!GetMazeCell(maze, &poss[i])->visited) break;
    }
    if (i == n) /* No unvisted neighbor */
    {
      if (pidx == 0) break;
      pidx--;
      continue;
    }
    if ((pidx + 1) == max_path) break;
    path[++pidx] = poss[i];
    visit(GetMazeCell(maze, &poss[i]));
  }

  if (PointsEqual(&path[pidx], dest))
  {
    return pidx + 1;
  }
  return 0;
}

/* - - Maze Internal API. - - */

static void CrawlMazeDrawing(maze_t *maze, maze_cell_t *start)
{
  point_t poss[4];
  size_t n, i;
  maze_cell_t *current, *next;
  maze_cell_pair_t *conn;
  priority_queue_t *conn_queue;
  conn_queue = CreatePriorityQueue();
  current = start;
  while (current)
  {
    visit(current);
    /* Create a list of potential neighbours. */
    for (i = 0; i < 4; i++) poss[i] = current->pos;
    n = 2;
    poss[0].col++;
    poss[1].row++;
    if (poss[n].col > 0) poss[n++].col--;
    if (poss[n].row > 0) poss[n++].row--;
    /* Randomly enqueue all of the neighbours */
    for (i = 0; i < n; i++)
    {
      next = GetMazeCell(maze, &poss[i]);
      if (!next || next->visited) continue;
      conn = CreateMazeCellPair(current, next);
      EnqueuePriority(conn_queue, rand(), conn);
    }
    /* Pop out a connection and make it */
    do
    {
      conn = PopTopPriority(conn_queue);
      if (conn)
      {
        current = conn->src;
        next = conn->dest;
        FreeMazeCellPair(conn);
      }
    }
    while (conn && next->visited);
    if (!conn) break;
    ConnectMazeCells(current, next);
    current = next;
  }
  FreePriorityQueue(conn_queue);
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

/* - - Maze Cell API - - */

void GetMazeCellPosition(maze_cell_t const *cell, point_t *pos)
{
  if (!cell || !pos) return;
  *pos = cell->pos;
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
  if (cell->up) neighbours[i++] = cell->up->pos;
  if (cell->down) neighbours[i++] = cell->down->pos;
  if (cell->left) neighbours[i++] = cell->left->pos;
  if (cell->right) neighbours[i++] = cell->right->pos;
  return i;
}

/* - - Maze Cell Internal API. - - */

static maze_cell_t *CreateMazeCell(point_t const *pos)
{
  maze_cell_t *cell;
  if (!pos) return NULL;
  cell = (maze_cell_t*)calloc(1, sizeof(maze_cell_t));
  cell->pos = *pos;
  return cell;
}

static void FreeMazeCell(maze_cell_t *cell)
{
  if (!cell) return;
  memset(cell, 0, sizeof(maze_cell_t));
  free(cell);
}

static void ConnectMazeCells(maze_cell_t *a, maze_cell_t *b)
{
  if (!a || !b) return;
  if (PointsEqual(&a->pos, &b->pos)) return;
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

/* - - Maze Cell Pair API - - */

maze_cell_pair_t *CreateMazeCellPair(maze_cell_t *src, maze_cell_t *dest)
{
  maze_cell_pair_t *pair;
  pair = calloc(1, sizeof(maze_cell_pair_t));
  pair->src = src;
  pair->dest = dest;
  return pair;
}

void FreeMazeCellPair(maze_cell_pair_t *pair)
{
  memset(pair, 0, sizeof(maze_cell_pair_t));
  free(pair);
}
