#ifndef _MAZE_H_
#define _MAZE_H_

#include "common.h"

typedef struct maze_st maze_t;
typedef struct maze_cell_st maze_cell_t;
typedef struct {
  maze_cell_t *src;
  maze_cell_t *dest;
} maze_cell_pair_t;

typedef size_t maze_flag_t;
#define MAX_MAZE_FLAG 8
typedef size_t maze_property_t;
#define MAX_MAZE_PROPERTY 8

/* - - Maze - - */
maze_t *CreateMaze(size_t height, size_t width, point_t const *start, point_t const *end);
void FreeMaze(maze_t *maze);

void ReDrawMaze(maze_t *maze, point_t const *start, point_t const *end);

size_t MazeWidth(maze_t const *maze);
size_t MazeHeight(maze_t const *maze);
void MazeStart(maze_t const *maze, point_t *pos);
void MazeEnd(maze_t const *maze, point_t *pos);

maze_cell_t *GetMazeCell(maze_t const *maze, point_t const *pos);
maze_cell_t *GetMazeStartCell(maze_t const *maze);
maze_cell_t *GetMazeEndCell(maze_t const *maze);

/*
 *  Gets path from point A to B.  Returns 0 if no path exists.
 *  If A and B are the same point, then 1 path element is returned.
 */
size_t ComputeMazePath(
  maze_t const *maze, point_t const *a, point_t const *b,
  point_t *path, size_t max_path);


/* - - Maze Cell - - */
void GetMazeCellPosition(maze_cell_t const *cell, point_t *pos);

/* Flags */
bool_t GetMazeCellFlag(maze_cell_t const *cell, maze_flag_t flag);
void SetMazeCellFlag(maze_cell_t *cell, maze_flag_t flag, bool_t value);

/* Properties */
int64_t GetMazeCellProperty(maze_cell_t const *cell, maze_property_t property);
void SetMazeCellProperty(maze_cell_t *cell, maze_property_t property, int64_t value);
void IncMazeCellProperty(maze_cell_t *cell, maze_property_t property);
void DecMazeCellProperty(maze_cell_t *cell, maze_property_t property);

/* neighbours must be large enough to fit 4 points */
size_t GetMazeCellNeighbourPoints(maze_cell_t const *cell, point_t *neightbours);

/* - - Maze Cell Pair - - */
maze_cell_pair_t *CreateMazeCellPair(maze_cell_t *src, maze_cell_t *dest);
void FreeMazeCellPair(maze_cell_pair_t *pair);

#endif /* _MAZE_H_ */
