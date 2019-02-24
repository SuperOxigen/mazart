
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <png.h>

#include "grid.h"
#include "maze_image.h"

#define BIT_DEPTH 8
static uint32_t const kOneKiloByte = 1024;

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

static bool_t PositionsAreAdjacent(point_t const *a, point_t const *b);

/* - - Maze Image API - - */

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

void DrawPathOnMazeImage(
  maze_image_t *image,
  point_t const *path, size_t path_length,
  rgb_t const *color)
{
  point_t ipos;
  size_t i;
  if (!image || !path || !color) return;
  for (i = 0; i < path_length; i++)
  {
    /* Fill cell */
    MazePositionToMazeImagePosition(image, &path[i], &ipos);
    DrawRectangle(
      image, &ipos,
      image->config.cell_width, image->config.cell_width,
      color);
    /* Fill connection */
    if (i > 0 && image->config.wall_width > 0)
    {
      /* Can only fill connections of consecutive points are adjacent. */
      if (!PositionsAreAdjacent(&path[i], &path[i - 1])) continue;

      /* DrawRectangle: h, w */
      if (path[i].row < path[i - 1].row)
      {
        /* Case: Current cell is a row below */
        ipos.row += image->config.cell_width;
        DrawRectangle(
          image, &ipos,
          image->config.wall_width, image->config.cell_width,
          color);
      }
      else if (path[i].row > path[i - 1].row)
      {
        /* Case: Current cell is a row above */
        ipos.row -= image->config.wall_width;
        DrawRectangle(
          image, &ipos,
          image->config.wall_width, image->config.cell_width,
          color);
      }
      else if (path[i].col < path[i - 1].col)
      {
        /* Case: Current cell is a column below */
        ipos.col += image->config.cell_width;
        DrawRectangle(
          image, &ipos,
          image->config.cell_width, image->config.wall_width,
          color);
      }
      else
      {
        /* Case: Current cell is a column above */
        ipos.col -= image->config.wall_width;
        DrawRectangle(
          image, &ipos,
          image->config.cell_width, image->config.wall_width,
          color);
      }
    }
  }
}

#define EXPORT_EXIT(clean_up_state) exit_on_cleanup = true; goto clean_up_state

void ExportImageToPNG(maze_image_t const *image, char const *png_path)
{
  FILE *out_fp;
  png_structp png_ptr;
  png_infop info_ptr;
  bool exit_on_cleanup;
  png_time cur_time;
  time_t sys_time;
  point_t ipos;
  png_byte *row_data, **rows_data;
  rgb_t const *pixel_color;
  uint8_t *red_ptr, *green_ptr, *blue_ptr;
  size_t width, height, row_count, i;
  if (!image || !png_path)
  {
    fprintf(stderr, "Missing image or png_path\n");
    EXPORT_EXIT(CLEAN_UP_END);
  }

  png_ptr = NULL;
  info_ptr = NULL;
  row_data = NULL;
  exit_on_cleanup = false;

  out_fp = fopen(png_path, "wb");
  if (!out_fp)
  {
    fprintf(stderr, "Failed to open PNG export file %s\n", png_path);
    EXPORT_EXIT(CLEAN_UP_END);
  }

  png_ptr = png_create_write_struct(
    PNG_LIBPNG_VER_STRING,
    NULL /* Error callback argument */,
    NULL /* Error callback function */,
    NULL /* Warning callback function */);
  if (!png_ptr)
  {
    fprintf(stderr, "Failed to create PNG structure\n");
    EXPORT_EXIT(CLEAN_UP_FILE);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    fprintf(stderr, "Failed to create PNG info structure\n");
    EXPORT_EXIT(CLEAN_UP_PNG_STRUCT);
  }

  /* PNG Meta Data */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    fprintf(stderr, "Failed to set PNG Meta Data\n");
    EXPORT_EXIT(CLEAN_UP_PNG_STRUCT);
  }

  width = MazeImageWidth(image);
  height = MazeImageHeight(image);
  png_init_io(png_ptr, out_fp);
  png_set_IHDR(png_ptr, info_ptr,
    width, height,
    BIT_DEPTH /* Bit Depth */,
    PNG_COLOR_TYPE_RGB /* Color Type */,
    PNG_INTERLACE_NONE /* Interlace Type */,
    PNG_COMPRESSION_TYPE_DEFAULT /* Compression Type */,
    PNG_FILTER_TYPE_DEFAULT /* Filter Type */);
  /* Set PNG time */
  time(&sys_time);
  png_convert_from_struct_tm(&cur_time, gmtime(&sys_time));
  png_set_tIME(png_ptr, info_ptr, &cur_time);

  png_set_flush(png_ptr, (16 * kOneKiloByte) / (4 * width));
  png_write_info(png_ptr, info_ptr);

  /* PNG Image Data */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    fprintf(stderr, "Failed to set PNG Image Data\n");
    EXPORT_EXIT(CLEAN_UP_ROW_DATA);
  }

  row_count = 0;  /* Set this now incase of a jump to clean up. */
  rows_data = (png_byte**)calloc(height, sizeof(png_byte*));
  if (!rows_data)
  {
    fprintf(
      stderr,
      "Failed to allocate row array data buffer (%lu height, %lu byte size)\n",
      height, (height * sizeof(png_byte*)));
    EXPORT_EXIT(CLEAN_UP_PNG_STRUCT);
  }

  for (ipos.row = 0; ipos.row < height; ipos.row++, row_count++)
  {
    row_data = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));
    if (!row_data)
    {
      fprintf(
        stderr,
        "Failed to allocate row %lu data buffer (%lu width, %lu byte size)\n",
        row_count, width, png_get_rowbytes(png_ptr, info_ptr));
      EXPORT_EXIT(CLEAN_UP_ROW_DATA);
    }
    rows_data[ipos.row] = row_data;
    for (ipos.col = 0; ipos.col < width; ipos.col++)
    {
      red_ptr = (uint8_t *) &row_data[ipos.col * 3];
      green_ptr = (uint8_t *) &row_data[ipos.col * 3 + 1];
      blue_ptr = (uint8_t *) &row_data[ipos.col * 3 + 2];
      pixel_color = GetGridCell(image->pixels, &ipos);
      if (!pixel_color)
      {
        pixel_color = &image->config.default_wall_color;
      }
      *red_ptr = pixel_color->red;
      *green_ptr = pixel_color->green;
      *blue_ptr = pixel_color->blue;
    }
  }
  png_write_image(png_ptr, rows_data);
  png_write_end(png_ptr, NULL);

CLEAN_UP_ROW_DATA:
  if (rows_data)
  {
    for (i = 0; i < row_count; i++)
    {
      free(rows_data[i]);
      rows_data[i] = NULL;
    }
    free(rows_data);
    rows_data = NULL;
  }
CLEAN_UP_PNG_STRUCT:
  if (png_ptr)
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    png_ptr = NULL;
    info_ptr = NULL;
  }
CLEAN_UP_FILE:
  if (out_fp)
  {
    fclose(out_fp);
    out_fp = NULL;
  }
CLEAN_UP_END:
  if (exit_on_cleanup)
  {
    exit(EXIT_FAILURE);
  }
}

/* - - Maze Image Internal - - */

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
  /* Reuse old color object if available. */
  cur_color = GetGridCell(image->pixels, pos);
  if (cur_color)
  {
    *cur_color = *color;
    return;
  }
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

static bool_t PositionsAreAdjacent(point_t const *a, point_t const *b)
{
  /* Check that they share a row or a column */
  if (a->row != b->row && a->col != b->col) return false;
  return (a->row + 1 == b->row || a->row == b->row + 1
       || a->col + 1 == b->col || a->col == b->col + 1);
}
