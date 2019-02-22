#ifndef _GRID_H_
#define _GRID_H_

#include "common.h"

typedef struct grid_st grid_t;

grid_t *CreateGrid(size_t height, size_t width);
void FreeGrid(grid_t *grid);

void *GetGridCell(grid_t const *grid, point_t const *pos);
bool_t SetGridCell(grid_t *grid, point_t const *pos, void *cell);

size_t GridWidth(grid_t const *grid);
size_t GridHeight(grid_t const *grid);

void ClearGrid(grid_t *grid);
void ClearGridFreeCells(grid_t *grid);
void ClearGridDestroyCell(grid_t *grid, void (*dtor)(void *));

#endif /* _GRID_H_ */
