#ifndef _MAZE_IMAGE_H_
#define _MAZE_IMAGE_H_

#include "color.h"
#include "common.h"
#include "maze.h"

/* Maze Image Config */
typedef bool_t (*cell_to_color_t)(maze_cell_t const*, rgb_t *);
typedef bool_t (*cell_connection_to_color_t)(maze_cell_t const*, maze_cell_t const*, rgb_t *);

typedef struct {
  /* Thicknesses */
  size_t border_width;
  size_t cell_width;
  size_t wall_width;
  /* Color generators */
  cell_to_color_t cell_color_gen;
  cell_connection_to_color_t conn_color_gen;
  /* Default color */
  rgb_t default_cell_color;
  rgb_t default_wall_color;
  rgb_t default_conn_color;
  rgb_t default_border_color;
  rgb_t default_path_color;
} maze_image_config_t;
void ClearMazeImageConfig(maze_image_config_t *config);
void DefaultMazeImageConfig(maze_image_config_t *config);

typedef struct maze_image_st maze_image_t;

maze_image_t *CreateMazeImage(
  maze_t const *maze, maze_image_config_t const *config);
void FreeMazeImage(maze_image_t *image);

size_t MazeImageWidth(maze_image_t const *image);
size_t MazeImageHeight(maze_image_t const *image);

void DrawPathOnMazeImage(
  maze_image_t *image,
  point_t const *path, size_t path_length,
  rgb_t const *path_color);

void ExportImageToPNG(maze_image_t const *image, char const *png_path);


#endif /* _MAZE_IMAGE_H_ */
