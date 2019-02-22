
#include "grid.h"
#include <stdlib.h>
#include <string.h>

struct grid_st {
  void ***data;  /* [row][col]->cell */
  size_t width;
  size_t height;
};

grid_t *CreateGrid(size_t height, size_t width)
{
  size_t i;
  grid_t *grid;
  if (height == 0 || width == 0) return NULL;
  grid = calloc(1, sizeof(grid_t));
  grid->height = height;
  grid->width = width;
  grid->data = (void***)calloc(height, sizeof(void**));
  for (i = 0; i < height; i++)
  {
    grid->data[i] = (void**)calloc(width, sizeof(void*));
  }
  return grid;
}

void FreeGrid(grid_t *grid)
{
  size_t i;
  if (!grid) return;
  ClearGrid(grid);
  for (i = 0; i < grid->height; i++)
  {
    if (!grid->data[i]) continue;
    free(grid->data[i]);
    grid->data[i] = NULL;
  }
  free(grid->data);
  memset(grid, 0, sizeof(grid_t));
  free(grid);
}

void *GetGridCell(grid_t const *grid, size_t row, size_t col)
{
  if (!grid) return NULL;
  if (row >= grid->height || col >= grid->width) return NULL;
  return grid->data[row][col];
}

bool_t SetGridCell(grid_t *grid, size_t row, size_t col, void *cell)
{
  if (!grid) return false;
  if (row >= grid->height || col >= grid->width) return false;
  grid->data[row][col] = cell;
  return true;
}

size_t GridWidth(grid_t const *grid)
{
  if (!grid) return 0;
  return grid->width;
}

size_t GridHeight(grid_t const *grid)
{
  if (!grid) return 0;
  return grid->height;
}

static void nopFree(void * v __unused)
{
  return;
}

void ClearGrid(grid_t *grid)
{
  ClearGridDestroyCell(grid, nopFree);
}

void ClearGridFreeCells(grid_t *grid)
{
  ClearGridDestroyCell(grid, free);
}

void ClearGridDestroyCell(grid_t *grid, void (*dtor)(void *))
{
  size_t i, j;
  if (!grid || !dtor) return;
  for (i = 0; i < grid->height; i++)
  {
    for (j = 0; j < grid->width; j++)
    {
      dtor(grid->data[i][j]);
      grid->data[i][j] = NULL;
    }
  }
}