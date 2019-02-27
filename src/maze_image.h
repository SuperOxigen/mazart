/*
 * Mazart - Maze Image
 *  Module provides an easy method of converting a Maze object into a
 *  RGB Color bit map image of the Maze.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _MAZE_IMAGE_H_
#define _MAZE_IMAGE_H_

#include "color.h"
#include "common.h"
#include "maze.h"

/* - - Maze Image Config - - */

/*
 * Cell to Color function type.
 *  Provided functions are called to determine the color of the cell
 *  area in the image.
 * Parameters:
 *  (void*)
 *    - Generator context.  This can a pointer to some contextual data
 *      that is shared between all calls to the function.  Value is
 *      provided by the programmer in the config's cell_color_ctx
 *      variable.  Maybe NULL.
 *  (maze_cell_t const *)
 *    - A pointer to the Maze Cell which is to be colored.
 *  (rgb_t *) [output]
 *    - A pointer to a RGB Color object which the function is to assign
 *      a color too.  The pre-call value of the color might be
 *      unitialized, and should always be assigned a value if function
 *      returns true.
 * Results:
 *    - Return true to let the caller know to use the color found in
 *      the Color parameter.  Return false if the default cell color
 *      is to be uesd.
 */
typedef bool_t (*cell_to_color_t)(void *, maze_cell_t const*, rgb_t *);
/*
 * Cell Connection to Color function type.
 *  Provided functions are called to determine the color of the area
 *  between two connected cells in the image.
 * Parameters:
 *  (void*)
 *    - Generator context.  This can a pointer to some contextual data
 *      that is shared between all calls to the function.  Value is
 *      provided by the programmer in the config's conn_color_ctx
 *      variable.  Maybe NULL.
 *  (maze_cell_t const *, maze_cell_t const *)
 *    - A pointer to two of the connected cells.  No cell order of the
 *      two cells should be assumed.
 *  (rgb_t *) [output]
 *    - A pointer to a RGB Color object which the function is to assign
 *      a color too.  The pre-call value of the color might be
 *      unitialized, and should always be assigned a value if function
 *      returns true.
 * Results:
 *    - Return true to let the caller know to use the color found in
 *      the Color parameter.  Return false if the default cell color
 *      is to be uesd.
 */
typedef bool_t (*cell_connection_to_color_t)(
  void *, maze_cell_t const*, maze_cell_t const*, rgb_t *);

/*
 * Maze Image Config Struct
 *  Contains all the basic configuration settings supported by the Maze
 *  Image module.
 * Terms:
 *  Cell - the maze grid cell.
 *  Wall - space between cells that are not connected.
 *  Border - space surrounding the maze.
 */
typedef struct {
  /* Thicknesses (in pixels).  All widths must be less than 128. */
  size_t cell_width; /* Cannot be 0 */
  size_t wall_width;
  size_t border_width;
  /* Fixed colors */
  rgb_t wall_color;
  rgb_t border_color;
  /* Color generators.  See notes above for details. */
  cell_to_color_t cell_color_gen;
  void *cell_color_ctx;
  cell_connection_to_color_t conn_color_gen;
  void *conn_color_ctx;
  /* Default color */
  rgb_t default_cell_color;
  rgb_t default_conn_color;
  rgb_t default_path_color;
} maze_image_config_t;

/* Clears all values of the config struct. */
void ClearMazeImageConfig(maze_image_config_t *config);
/* Clears and sets all the values of the config struct to their default
 * value.  This function should be called on the config struct first,
 * then assigned alternative values.. */
void DefaultMazeImageConfig(maze_image_config_t *config);

/* - - Maze Image - - */

typedef struct maze_image_st maze_image_t;

/* Maze Image constructor.  This will create the Maze Image object and
 * convert the provided Maze into an image using the Config settings.
 * If the Config is NULL, then the default values are used.
 * No ownership of the provided Maze or Config is made, pointers are
 * not retained internally.
 */
maze_image_t *CreateMazeImage(
  maze_t const *maze, maze_image_config_t const *config);
/* Maze Image destructor.  This will free all of the Maze Image
 * resources, but it will have no effect on the original Maze
 * object the image was created from. */
void FreeMazeImage(maze_image_t *image);

/* Maze Image dimension getters.  This is the height and width of the
 * created image in pixels. */
size_t MazeImageHeight(maze_image_t const *image);
size_t MazeImageWidth(maze_image_t const *image);

/*
 * Draw Path on Maze Image
 *  Does exactly as it says.  This function assumes that consecutive
 *  positions provided in `path` are adjacent.  If they are not, then
 *  no connection is made between the points.
 *  if `path_color` is NULL, then the default path color in the
 *  original config is used.
 */
void DrawPathOnMazeImage(
  maze_image_t *image,
  point_t const *path, size_t path_length,
  rgb_t const *path_color);

#ifndef _NO_PNG
/*
 * Exports a Maze Image to a file in PNG format.
 *  Uses LibPNG to convert the Maze Image's pixel bit-map to a PNG
 *  file that is specified by the provided `png_path`.
 *
 * Basic PNG settings:
 *  Bit Depth: 8 bits
 *  Color Type: RGB
 *  Interlace: none
 *  Compression Type: LibPNG Default
 *  Filter Type: LibPNG Default
 *
 * IMPORTANT: unlike most functions in this program, this one will exit
 *  the program on error and provide some information as to why to
 *  standard error.
 */
void ExportMazeImageToPNG(maze_image_t const *image, char const *png_path);
#endif /* _NO_PNG */


#endif /* _MAZE_IMAGE_H_ */
