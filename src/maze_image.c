
#include <stdlib.h>
#include <string.h>

#include "grid.h"
#include "maze_image.h"

/* - - Maze Image Config - - */
void ClearMazeImageConfig(maze_image_config_t *config)
{
  if (!config) return;
  memset(config, 0, sizeof(maze_image_config_t));
}

void DefaultMazeImageConfig(maze_image_config_t *config)
{
  if (!config) return;
  ClearMazeImageConfig(config);
  config->border_width = 5;
  config->cell_width = 4;
  config->wall_width = 2;

  config->default_cell_color = /* White */ (rgb_t) {
    .red = 255,
    .blue = 255,
    .green = 255
  };
  config->default_conn_color = /* Light Grey */ (rgb_t) {
    .red = 225,
    .blue = 225,
    .green = 225
  };
  config->default_path_color = /* Red */ (rgb_t) {
    .red = 255,
    .blue = 0,
    .green = 0
  };
}

static bool_t IsConfigValid(maze_image_config_t const *config)
{
  if (!config) return false;
  if (config->cell_width < 1) return false;
  return true;
}

static inline void CopyConfig(maze_image_config_t const *src, maze_image_config_t *dest)
{
  if (!src || !dest) return;
  memcpy(dest, src, sizeof(maze_image_config_t));
}

/* - - Maze Image - - */

struct maze_image_st {
  grid_t *pixels;
  maze_image_config_t config;
};

static void DrawMazeImageBorders(maze_image_t *image);
static void DrawMazeImageCells(maze_image_t *image, maze_t const *maze);
static void FillEmptyMazeImageCells(maze_image_t *image, rgb_t const *color);

static void DrawRectangle(maze_image_t *image, point_t const *corner, size_t height, size_t width, rgb_t const *color);
static void MazePositionToMazeImagePosition(maze_image_t *image, point_t const *maze_pos, point_t *image_pos);

/* Sets a image cell color and frees the previous color if necessary. */
static void SetMazeImageCellColor(maze_image_t *image, point_t const *pos, rgb_t const *color);
static void SetMazeImageCellColorIfUnset(maze_image_t *image, point_t const *pos, rgb_t const *color);

static inline void GetCellColor(maze_image_t const *image, maze_cell_t const *cell, rgb_t *color)
{
  if (image->config.cell_color_gen && image->config.cell_color_gen(cell, color)) return;
  *color = image->config.default_cell_color;
}

static inline void GetConnColor(maze_image_t const *image, maze_cell_t const *a, maze_cell_t const *b, rgb_t *color)
{
  if (image->config.conn_color_gen && image->config.conn_color_gen(a, b, color)) return;
  *color = image->config.default_conn_color;
}

maze_image_t *CreateMazeImage(maze_t const *maze, maze_image_config_t const *config)
{
  maze_image_t *image;
  size_t width, height;
  if (!maze) return NULL;
  if (config && !IsConfigValid(config)) return NULL;
  if (MazeHeight(maze) == 0 || MazeWidth(maze) == 0) return NULL;

  image = (maze_image_t*)calloc(1, sizeof(maze_image_t));
  if (config) CopyConfig(config, &image->config);
  else DefaultMazeImageConfig(&image->config);

  width = (MazeWidth(maze) * image->config.cell_width)
    + ((MazeWidth(maze) - 1) * image->config.wall_width)
    + (image->config.border_width * 2);
  height = (MazeHeight(maze) * image->config.cell_width)
    + ((MazeHeight(maze) - 1) * image->config.wall_width)
    + (image->config.border_width * 2);
  image->pixels = CreateGrid(height, width);

  DrawMazeImageBorders(image);
  DrawMazeImageCells(image, maze);
  FillEmptyMazeImageCells(image, &image->config.default_wall_color);
  return image;
}

void FreeMazeImage(maze_image_t *image)
{
  if (!image) return;
  ClearGridDestroyCell(image->pixels, FreeVoidColor);
  FreeGrid(image->pixels);
  memset(image, 0, sizeof(maze_image_t));
  free(image);
}

size_t MazeImageWidth(maze_image_t const *image)
{
  if (!image) return 0;
  return GridWidth(image->pixels);
}

size_t MazeImageHeight(maze_image_t const *image)
{
  if (!image) return 0;
  return GridHeight(image->pixels);
}

static void DrawMazeImageBorders(maze_image_t *image)
{
  point_t pos;
  size_t width, height, thickness;
  thickness = image->config.border_width;
  if (thickness == 0) return;
  width = MazeImageWidth(image);
  height = MazeImageHeight(image);
  /* Top */
  for (pos.row = 0; pos.row < thickness; pos.row++)
  {
    for (pos.col = 0; pos.col < width; pos.col++)
    {
      SetMazeImageCellColor(image, &pos, &image->config.default_border_color);
    }
  }
  /* Bottom */
  for (pos.row = height - thickness; pos.row < height; pos.row++)
  {
    for (pos.col = 0; pos.col < width; pos.col++)
    {
      SetMazeImageCellColor(image, &pos, &image->config.default_border_color);
    }
  }
  /* Sides */
  for (pos.row = thickness; pos.row < (height - thickness); pos.row++)
  {
    for (pos.col = 0; pos.col < thickness; pos.col++)
    {
      SetMazeImageCellColor(image, &pos, &image->config.default_border_color);
    }
    for (pos.col = width - thickness; pos.col < width; pos.col++)
    {
      SetMazeImageCellColor(image, &pos, &image->config.default_border_color);
    }
  }
}

static void FillEmptyMazeImageCells(maze_image_t *image, rgb_t const *color)
{
  point_t pos;
  size_t width, height;
  width = MazeImageWidth(image);
  height = MazeImageHeight(image);
  for (pos.row = 0; pos.row < height; pos.row++)
  {
    for (pos.col = 0; pos.col < width; pos.col++)
    {
      SetMazeImageCellColorIfUnset(image, &pos, color);
    }
  }
}

static void DrawMazeImageCells(maze_image_t *image, maze_t const *maze)
{
  point_t mpos, ipos;
  point_t poss[4];
  rgb_t color;
  maze_cell_t *cell, *neighbour;
  size_t n, i, mheight, mwidth;
  mheight = MazeHeight(maze);
  mwidth = MazeWidth(maze);
  for (mpos.row = 0; mpos.row < mheight; mpos.row++)
  {
    for (mpos.col = 0; mpos.col < mwidth; mpos.col++)
    {
      cell = GetMazeCell(maze, &mpos);
      if (!cell) continue;
      /* Draw cell */
      GetCellColor(image, cell, &color);
      MazePositionToMazeImagePosition(image, &mpos, &ipos);
      DrawRectangle(
        image, &ipos,
        image->config.cell_width, image->config.cell_width,
        &color);
      /* Draw connections */
      if (image->config.wall_width == 0) continue;
      n = GetMazeCellNeighbourPoints(cell, poss);
      for (i = 0; i < n; i++)
      {
        /* Skips up or back connections*/
        if (poss[i].row < mpos.row || poss[i].col < mpos.col) continue;
        neighbour = GetMazeCell(maze, &poss[i]);
        GetConnColor(image, cell, neighbour, &color);
        MazePositionToMazeImagePosition(image, &mpos, &ipos);
        if (poss[i].row == mpos.row)  /* Row Connection */
        {
          ipos.col += image->config.cell_width;
          DrawRectangle(
            image, &ipos,
            image->config.cell_width, image->config.wall_width,
            &color);
        }
        else /* Col Connection */
        {
          ipos.row += image->config.cell_width;
          DrawRectangle(
            image, &ipos,
            image->config.wall_width, image->config.cell_width,
            &color);
        }
      }
    }
  }
}

static void DrawRectangle(maze_image_t *image, point_t const *corner, size_t height, size_t width, rgb_t const *color)
{
  point_t pos;
  for (pos.row = corner->row; pos.row < (corner->row + height); pos.row++)
  {
    for (pos.col = corner->col; pos.col < (corner->col + width); pos.col++)
    {
      SetMazeImageCellColor(image, &pos, color);
    }
  }
}

static void MazePositionToMazeImagePosition(maze_image_t *image, point_t const *maze_pos, point_t *image_pos)
{
  image_pos->row = (image->config.cell_width + image->config.wall_width) * maze_pos->row + image->config.border_width;
  image_pos->col = (image->config.cell_width + image->config.wall_width) * maze_pos->col + image->config.border_width;
}

/* Sets a image cell color and frees the previous color if necessary. */
static void SetMazeImageCellColor(maze_image_t *image, point_t const *pos, rgb_t const *color)
{
  rgb_t *cur_color, *new_color;
  /* Clean up old color. */
  cur_color = GetGridCell(image->pixels, pos);
  if (cur_color) FreeColor(cur_color);
  /* Create new color. */
  new_color = CloneColor(color);
  /* Possible setting color fails if pos is out of range incorrect. */
  if (!SetGridCell(image->pixels, pos, new_color)) FreeColor(new_color);
}

static void SetMazeImageCellColorIfUnset(maze_image_t *image, point_t const *pos, rgb_t const *color)
{
  rgb_t *cur_color, *new_color;
  /* Check for old color. */
  cur_color = GetGridCell(image->pixels, pos);
  if (cur_color) return;
  /* Create new color. */
  new_color = CloneColor(color);
  /* Possible setting color fails if pos is out of range incorrect. */
  if (!SetGridCell(image->pixels, pos, new_color)) FreeColor(new_color);
}
