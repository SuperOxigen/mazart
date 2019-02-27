/*
 * Mazart - Grid
 *  Module provides a 2D grid for storing generic objects.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _GRID_H_
#define _GRID_H_

#include "common.h"

/*
 * Grid Struct
 *  Provides a simple interface for accessing elements in a 2D array.
 *  The interface is designed to protect against out of bounds errors.
 *  If objects assigned to the grid which are intended for the grid to
 *  own, the programmer must ensure an appropriate call to
 *  ClearGridDestroyCells() and provide a non-NULL destructor.
 *
 *  Grid cannot be zero-sized.
 */
typedef struct grid_st grid_t;

/* - - Grid API - - */

/* Grid constructor - Height and width must be non-zero. */
grid_t *CreateGrid(size_t height, size_t width);
/* FreeGrid() *will not* free the stored elements. */
void FreeGrid(grid_t *grid);

/* Grid cell getter and setter.   Returns NULL or false if the provided
 * position is out of bounds. */
void *GetGridCell(grid_t const *grid, point_t const *pos);
bool_t SetGridCell(grid_t *grid, point_t const *pos, void *cell);

/* Grid dimension getters. Same values provided in CreateGrid(). */
size_t GridHeight(grid_t const *grid);
size_t GridWidth(grid_t const *grid);

/* Sets all grid cells to NULL, no action is made on the element. */
void ClearGrid(grid_t *grid);
/* Sets all grid cells to NULL, calling free() on any non-null cell
 * elements. */
void ClearGridFreeCells(grid_t *grid);
/* Sets all grid cells to NULL, calling the provided dtor() on any
 * non-null cell elements.  The destructor must be non-NULL. */
void ClearGridDestroyCells(grid_t *grid, void (*dtor)(void *));

#endif /* _GRID_H_ */
