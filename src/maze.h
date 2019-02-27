/*
 * Mazart - Maze
 *  Module provides a Maze generating object.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _MAZE_H_
#define _MAZE_H_

#include "common.h"

/* - - Maze and Maze Cell Handles - - */

/*
 * Maze Struct
 *  A 2D Maze object that contains a randomly generated Maze conisiting
 *  of a Grid of Maze Cells.
 */
typedef struct maze_st maze_t;

/*
 * Maze Cell Struct
 *  An individual Grid cell of a Maze objects.  Can be querried for
 *  neighbouring Maze Cells (cells that are adjacent and connected).
 *  A Maze Cell contains general use Boolean flags and signed integer
 *  properties which can be used by applications for tracking
 *  information about a Maze Cell in the Maze Cell.
 */
typedef struct maze_cell_st maze_cell_t;

/* - - Maze API - - */
/* Maze constructor.  Generates a Maze using the given dimensions,
 * starting at the provided starting point.  The end point is stored,
 * but only used when calculating the solution.  The start and end
 * points are copied into the internal structure of Maze, and the
 * original pointers are not stored.
 *
 * Maze width and height cannot be 0, and the starting and end point
 * must exist within the Maze bounds.
 *
 * The Maze object will constract and own Maze Cells which can be
 * accessed by reference using the appropriate GetMazeCell() call.
 */
maze_t *CreateMaze(size_t height, size_t width, point_t const *start, point_t const *end);
/* Maze destructor.  This will free all Maze Cells and other internal
 * Maze resources.  All external references to Maze Cells should
 * treated as dead pointers. */
void FreeMaze(maze_t *maze);

/* Clears and redraws a new maze.  The `start` and `end` points can
 * optionally be updated, otherwise, the same start and end points
 * are used.  Regerences to Maze Cells remain valid and will not
 * change internal position. */
void ReDrawMaze(maze_t *maze, point_t const *start, point_t const *end);

/* Maze dimension getters. */
size_t MazeHeight(maze_t const *maze);
size_t MazeWidth(maze_t const *maze);
/* Maze start and end position getters. */
void MazeStart(maze_t const *maze, point_t *pos);
void MazeEnd(maze_t const *maze, point_t *pos);

/* Maze Cell getters. */
maze_cell_t *GetMazeCell(maze_t const *maze, point_t const *pos);
maze_cell_t *GetMazeStartCell(maze_t const *maze);
maze_cell_t *GetMazeEndCell(maze_t const *maze);

/* Gets path from point A to B.  Returns 0 if no path exists. If A and
 * B are the same point, then 1 path element is returned.  All
 * paramenters must be non-NULL. */
size_t ComputeMazePath(
  maze_t const *maze, point_t const *a, point_t const *b,
  point_t *path, size_t max_path);

/* - - Maze Cell - - */

/* Maze flag type is an numeric index to a user defined Boolean flag. */
typedef size_t maze_flag_t;
/* Maze property type is a numeric index to a user defined integer property. */
typedef size_t maze_property_t;

/* Max number of flags that are supported. */
#ifndef MAX_MAZE_FLAG
#define MAX_MAZE_FLAG 8
#endif /* MAX_MAZE_FLAG */
/* Maze number of properties that are supported. */
#ifndef MAX_MAZE_PROPERTY
#define MAX_MAZE_PROPERTY 8
#endif /* MAX_MAZE_PROPERTY */

/* Maze Cell position getter. */
void GetMazeCellPosition(maze_cell_t const *cell, point_t *pos);

/* Flags */
bool_t GetMazeCellFlag(maze_cell_t const *cell, maze_flag_t flag);
void SetMazeCellFlag(maze_cell_t *cell, maze_flag_t flag, bool_t value);

/* Properties */
int64_t GetMazeCellProperty(maze_cell_t const *cell, maze_property_t property);
void SetMazeCellProperty(
  maze_cell_t *cell, maze_property_t property, int64_t value);
void IncMazeCellProperty(maze_cell_t *cell, maze_property_t property);
void DecMazeCellProperty(maze_cell_t *cell, maze_property_t property);

/* Neighbours buffer must be large enough to fit 4 points */
size_t GetMazeCellNeighbourPoints(maze_cell_t const *cell, point_t *neightbours);

/* - - Maze Cell Pair - - */

/*
 * Maze Cell Pair Struct
 *  This structure is simply a pair of Maze Cells.  They are intended to
 *  represent the connection between adjacent Maze Cell's, but this is
 *  not enforce.  This object is useful for Maze traverals.
 */
typedef struct {
  maze_cell_t *src;
  maze_cell_t *dest;
} maze_cell_pair_t;

/* Maze Cell Pair constructor. Simple constructor, only creates the Pair
 * object and directly assigns the provided Maze Cell pointers.  */
maze_cell_pair_t *CreateMazeCellPair(maze_cell_t *src, maze_cell_t *dest);
/* Maze Cell Pair destructor.  Releases the Pair object, not the Maze
 * Cells them selves.. */
void FreeMazeCellPair(maze_cell_pair_t *pair);

#endif /* _MAZE_H_ */
