/*
 * Mazart - Grid
 *  Module provides a 2D grid for storing generic objects.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "grid.h"

#include <stdlib.h>
#include <string.h>

/* - - Grid Structure - - */
struct grid_st {
  void ***data;  /* [row][col]->element */
  size_t height;
  size_t width;
};

/* - - Grid API - - */

grid_t *CreateGrid(size_t height, size_t width)
{
  size_t row;
  grid_t *grid;
  /* Zero-sized grids are non allowed. */
  if (height == 0 || width == 0) return NULL;
  grid = calloc(1, sizeof(grid_t));
  grid->height = height;
  grid->width = width;
  /* Create row pointers. */
  grid->data = (void***)calloc(height, sizeof(void**));
  /* Create rows. */
  for (row = 0; row < height; row++)
  {
    grid->data[row] = (void**)calloc(width, sizeof(void*));
  }
  return grid;
}

void FreeGrid(grid_t *grid)
{
  size_t row;
  if (!grid) return;
  ClearGrid(grid);
  for (row = 0; row < grid->height; row++)
  {
    if (!grid->data[row]) continue;  /* Technically, should not happend. */
    free(grid->data[row]);
    grid->data[row] = NULL;
  }
  free(grid->data);
  memset(grid, 0, sizeof(grid_t));
  free(grid);
}

/* - Cell Getters/Setters - */

static inline bool_t PositionIsGridBounded(
  grid_t const *grid,
  point_t const *pos)
{
  return pos->row < grid->height && pos->col < grid->width;
}

void *GetGridCell(grid_t const *grid, point_t const *pos)
{
  if (!grid || !pos) return NULL;
  if (!PositionIsGridBounded(grid, pos)) return NULL;
  return grid->data[pos->row][pos->col];
}

bool_t SetGridCell(grid_t *grid, point_t const *pos, void *cell)
{
  if (!grid) return false;
  if (!PositionIsGridBounded(grid, pos)) return false;
  grid->data[pos->row][pos->col] = cell;
  return true;
}

/* - Grid Dimensions - */

size_t GridHeight(grid_t const *grid)
{
  if (!grid) return 0;
  return grid->height;
}

size_t GridWidth(grid_t const *grid)
{
  if (!grid) return 0;
  return grid->width;
}

/* - Grid Clearing - */

static void nopFree(void * v __unused) { }

void ClearGrid(grid_t *grid)
{
  ClearGridDestroyCells(grid, nopFree);
}

void ClearGridFreeCells(grid_t *grid)
{
  ClearGridDestroyCells(grid, free);
}

void ClearGridDestroyCells(grid_t *grid, void (*dtor)(void *))
{
  size_t i, j;
  if (!grid || !dtor) return;
  for (i = 0; i < grid->height; i++)
  {
    for (j = 0; j < grid->width; j++)
    {
      /* Skip NULL cells. */
      if (!grid->data[i][j]) continue;
      dtor(grid->data[i][j]);
      grid->data[i][j] = NULL;
    }
  }
}
