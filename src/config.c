/*
 * Mazart - Program Configuration
 *  Parses commandline arguments and produces a set of configuration
 *  options.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "deque.h"

/* - - Flags and Default - - */

static char const kDebugModeFlag[] = "--debug";

static char const kMazeWidthFlag[] = "--maze-width";
static size_t const kMazeWidthMax = 2048;
static size_t const kMazeWidthDefault = 64;
static size_t const kMazeWidthMin = 8;

static char const kMazeHeightFlag[] = "--maze-height";
static size_t const kMazeHeightMax = 2048;
static size_t const kMazeHeightDefault = 64;
static size_t const kMazeHeightMin = 8;

static char const kSeedFlag[] = "--seed";
static char const kSeedDefaultName[] = "time";

static char const kCellWidthFlag[] = "--cell-width";
static size_t const kCellWidthMax = 64;
static size_t const kCellWidthDefault = 4;
static size_t const kCellWidthMin = 1;

static char const kWallWidthFlag[] = "--wall-width";
static size_t const kWallWidthDefault = 2;
static size_t const kWallWidthMax = 16;

static char const kBorderWidthFlag[] = "--border-width";
static size_t const kBorderWidthDefault = 8;
static size_t const kBorderWidthMax = 64;

static char const kCellColorFlag[] = "--cell-color";
static mazart_color_t const kCellColorDefault = CLR_WHITE;
static char const kCellColorDefaultName[] = "white";

static char const kCellColorMetricFlag[] = "--cell-metric";
static mazart_color_metric_t const kCellColorMetricDefault = CLR_MTRC_NONE;
static char const kCellColorMetricDefaultName[] = "none";

static char const kCellColorModeFlag[] = "--cell-mode";
static char const kCellColorModeDefault = CLR_MODE_NONE;
static char const kCellColorModeDefaultName[] = "none";

static char const kConnColorFlag[] = "--conn-color";
static mazart_color_t const kConnColorDefault = CLR_LIGHT_GREY;
static char const kConnColorDefaultName[] = "light-grey";

static char const kConnColorMethodFlag[] = "--conn-color-method";
static mazart_color_method_t const kConnColorMethodDefault = CLR_MTHD_FIXED;
static char const kConnColorMethodDefaultName[] = "fixed";

static char const kWallColorFlag[] = "--wall-color";
static mazart_color_t const kWallColorDefault = CLR_BLACK;
static char const kWallColorDefaultName[] = "black";

static char const kBorderColorFlag[] = "--boarder-color";
static mazart_color_t const kBorderColorDefault = CLR_OTHER;
static char const kBorderColorDefaultName[] = "(same as wall color)";

static char const kDrawPathFlag[] = "--draw-path";

static char const kPathColorFlag[] = "--path-color";
static mazart_color_t const kPathColorDefault = CLR_RED;
static char const kPathColorDefaultName[] = "red";

static char const kOutputFileFlag[] = "--output";

/* - - Config Consts - - */

#define STR_BUF_SZ 512
#define KSTRP_BUF_SZ 16

static size_t const kZero __unused;
static size_t const kZero = 0; /* User to prevent compiler warnings. */

static size_t const kFlagIndentSize = 30;
static size_t const kMaxTermWidth = 120;

static char const kColor[] = "COLOR";
typedef struct {
  kstring_t color_name;
  mazart_color_t color;
} known_color_t;
static known_color_t const kKnownColors[] = {
  {"white", CLR_WHITE},
  {"light-grey", CLR_LIGHT_GREY},
  {"grey", CLR_GREY},
  {"black", CLR_BLACK},
  {"blue", CLR_BLUE},
  {"teal", CLR_TEAL},
  {"green", CLR_GREEN},
  {"yellow", CLR_YELLOW},
  {"orange", CLR_ORANGE},
  {"red", CLR_RED},
  {"purple", CLR_PURPLE}
};
static size_t const kKnownColorsCount = sizeof(kKnownColors) / sizeof(kKnownColors[0]);

static char const kColorMetric[] = "METRIC";
typedef struct {
  kstring_t color_metric_name;
  mazart_color_metric_t color_metric;
} known_color_metric_t;
static known_color_metric_t const kKnownColorMetrics[] = {
  {"path", CLR_MTRC_PATH_DIST},
  {"start", CLR_MTRC_START_DIST},
  {"end", CLR_MTRC_END_DIST}
};
static size_t const kKnownColorMetricCount = sizeof(kKnownColorMetrics) / sizeof(kKnownColorMetrics[0]);

static char const kColorMode[] = "MODE";
typedef struct {
  kstring_t color_mode_name;
  mazart_color_mode_t color_mode;
} known_color_mode_t;
static known_color_mode_t const kKnownColorModes[] = {
  {"palette", CLR_MODE_PALETTE},
  {"preset-a", CLR_MODE_PRESET_A}
};
static size_t const kKnownColorModesCount = sizeof(kKnownColorModes) / sizeof(kKnownColorModes[0]);

static char const kColorMethod[] = "METHOD";
typedef struct {
  kstring_t color_method_name;
  mazart_color_method_t color_method;
} known_color_method_t;
static known_color_method_t kKnownColorMethods[] = {
  {"nearest", CLR_MTHD_NEAREST},
  {"average", CLR_MTHD_AVERAGE}
};
static size_t const kKnownColorMethodsCount = sizeof(kKnownColorMethods) / sizeof(kKnownColorMethods[0]);

static bool_t IsInteger(char const *value);
static size_t ParseInteger(char const *value);
static bool_t IsColor(char const *value);
static mazart_color_t ParseColor(char const *value);
static char const *ColorToString(mazart_color_t color);
static bool_t IsColorMetric(char const *value);
static mazart_color_metric_t ParseColorMetric(char const *value);
static char const *ColorMetricToString(mazart_color_metric_t metric);
static bool_t IsColorMode(char const *value);
static mazart_color_mode_t ParseColorMode(char const *value);
static char const *ColorModeToString(mazart_color_mode_t mode);
static bool_t IsColorMethod(char const *value);
static mazart_color_method_t  ParseConnColorMethod(char const *value);
static char const *ColorMethodToString(mazart_color_method_t method);
static bool_t IsFileName(char const *value);
static char *ParseFileName(char const *value);

static void PrintUsage(char const *prog);
static void PrintFlag(char const *flag, char const *description, char const *rep, char const *deflt);
static void PrintRangedFlag(char const *flag, char *description, char const *rep, size_t min, size_t max, size_t deflt);
static void PrintKnownValues(char const *value_name, kstring_t *values, size_t values_count);
static size_t PrintDescriptionColumn(char const *description, size_t indent);

static char *CopyString(char const *src_str);
static size_t AppendString(char *dest_str, char const *append_str, size_t dest_buf_size);
static bool_t StringsEqual(char const *a, char const *b);

static bool_t IsInteger(char const *value)
{
  char const *vptr;
  vptr = value;
  while (*vptr)
  {
    if (!isdigit(*vptr++)) return false;
  }
  return true;
}

static size_t ParseInteger(char const *value)
{
  size_t val;
  if (sscanf(value, "%lu", &val) != 1) return 0;
  return val;
}

static bool_t IsColor(char const *value)
{
  size_t i;
  if (!value) return false;
  for (i = 0; i < kKnownColorsCount; i ++)
  {
    if (StringsEqual(value, kKnownColors[i].color_name))
      return true;
  }
  return false;
}

static mazart_color_t ParseColor(char const *value)
{
  size_t i;
  if (!value) return CLR_NONE;
  for (i = 0; i < kKnownColorsCount; i ++)
  {
    if (StringsEqual(value, kKnownColors[i].color_name))
      return kKnownColors[i].color;
  }
  return CLR_NONE;
}

static char const *ColorToString(mazart_color_t color)
{
  size_t i;
  for (i = 0; i < kKnownColorsCount; i ++)
  {
    if (kKnownColors[i].color == color)
      return kKnownColors[i].color_name;
  }
  return "unknown";
}

static bool_t IsColorMetric(char const *value)
{
  size_t i;
  if (!value) return false;
  for (i = 0; i < kKnownColorMetricCount; i ++)
  {
    if (StringsEqual(value, kKnownColorMetrics[i].color_metric_name))
      return true;
  }
  return false;
}

static mazart_color_metric_t ParseColorMetric(char const *value)
{
  size_t i;
  if (!value) return CLR_MTRC_NONE;
  for (i = 0; i < kKnownColorMetricCount; i ++)
  {
    if (StringsEqual(value, kKnownColorMetrics[i].color_metric_name))
      return kKnownColorMetrics[i].color_metric;
  }
  return CLR_MTRC_NONE;
}

static char const *ColorMetricToString(mazart_color_metric_t metric)
{
  size_t i;
  for (i = 0; i < kKnownColorMetricCount; i ++)
  {
    if (kKnownColorMetrics[i].color_metric == metric)
      return kKnownColorMetrics[i].color_metric_name;
  }
  return "unknown";
}

static bool_t IsColorMode(char const *value)
{
  size_t i;
  if (!value) return false;
  for (i = 0; i < kKnownColorModesCount; i ++)
  {
    if (StringsEqual(value, kKnownColorModes[i].color_mode_name))
      return true;
  }
  return false;
}

static mazart_color_mode_t ParseColorMode(char const *value)
{
  size_t i;
  if (!value) return CLR_MODE_NONE;
  for (i = 0; i < kKnownColorModesCount; i ++)
  {
    if (StringsEqual(value, kKnownColorModes[i].color_mode_name))
      return kKnownColorModes[i].color_mode;
  }
  return CLR_MODE_NONE;
}

static char const *ColorModeToString(mazart_color_mode_t mode)
{
  size_t i;
  for (i = 0; i < kKnownColorModesCount; i ++)
  {
    if (kKnownColorModes[i].color_mode == mode)
      return kKnownColorModes[i].color_mode_name;
  }
  return "unknown";
}

static bool_t IsColorMethod(char const *value)
{
  size_t i;
  if (!value) return false;
  for (i = 0; i < kKnownColorMethodsCount; i ++)
  {
    if (StringsEqual(value, kKnownColorMethods[i].color_method_name))
      return true;
  }
  return false;
}

static mazart_color_method_t ParseConnColorMethod(char const *value)
{
  size_t i;
  if (!value) return CLR_MTRC_NONE;
  for (i = 0; i < kKnownColorMethodsCount; i ++)
  {
    if (StringsEqual(value, kKnownColorMethods[i].color_method_name))
      return kKnownColorMethods[i].color_method;
  }
  return CLR_MTRC_NONE;
}

static char const *ColorMethodToString(mazart_color_method_t method)
{
  size_t i;
  for (i = 0; i < kKnownColorMethodsCount; i ++)
  {
    if (kKnownColorMethods[i].color_method == method)
      return kKnownColorMethods[i].color_method_name;
  }
  if (method == CLR_MTHD_FIXED) return "fixed";
  return "unknown";
}

static bool_t IsFileName(char const *value)
{
  struct stat s;
  bool_t file_exists;
  int fd;
  if (!value) return false;
  file_exists = false;
  if (stat(value, &s) < 0)
  {
    switch (errno)
    {
      case EACCES:
        fprintf(stderr, "Error: Cannot access %s\n", value);
        return false;
      case EFAULT:
        fprintf(stderr, "Error: Invalid filename %s\n", value);
        return false;
      case ENOTDIR:
        fprintf(stderr, "Error: No directory of %s\n", value);
        return false;
      case ENOENT:
        break;  /* No file, might still be valid. */
      default:
        fprintf(stderr, "Error: stat(pathname = \"%s\", statbuf = %p), errno = %d\n", value, &s, (int) errno);
        return false;
    }
  }
  else  /* File exists */
  {
    if (!S_ISREG(s.st_mode))
    {
      fprintf(stderr, "Error: Not a regular file %s\n", value);
      return false;
    }
    file_exists = true;
  }
  fd = open(value, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd < 0)
  {
    switch (errno)
    {
      case EACCES:
        fprintf(stderr, "Error: Cannot create file %s\n", value);
        return false;
      default:
        fprintf(
          stderr,
          "Error: open(pathname = \"%s\", "
            "flags = O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR), "
            "errno = %d\n", value, (int) errno);
        return false;
    }
  }
  close(fd);
  if (!file_exists)
  {
    unlink(value);
  }
  return true;
}

static char *ParseFileName(char const *value)
{
  return CopyString(value);
}

static char *CopyString(char const *src_str)
{
  size_t len;
  char *str_copy;
  if (!src_str) return NULL;
  len = strlen(src_str);
  str_copy = calloc(len + 1, sizeof(char));
  strcpy(str_copy, src_str);
  return str_copy;
}

static size_t AppendString(char *dest_str, char const *append_str, size_t dest_buf_size)
{
  size_t i;
  char const *aptr;
  if (!dest_str || !append_str || dest_buf_size == 0) return 0;
  for (i = 0; i < dest_buf_size && dest_str[i]; i++) {}
  /* Check that dest buffer is not full (and cannot event support a NULL-term). */
  if (i == dest_buf_size) return 0;
  /* Start appending */
  for (aptr = append_str; *aptr && (i + 1) < dest_buf_size; i++, aptr++)
  {
    dest_str[i] = *aptr;
  }
  dest_str[i] = 0;
  /* Count how many additional bytes are required. */
  for (; *aptr; i++, aptr++) {}
  return i;
}

static bool_t StringsEqual(char const *a, char const *b)
{
  return  a && b && !strcmp(a, b);
}

static void PrintUsage(char const *prog)
{
  kstring_t buf[KSTRP_BUF_SZ];
  size_t i;
  printf("Usage: %s --output FILEPATH\n", prog ? prog : "mazart");

  printf("Required arguments:\n");
  PrintFlag(kOutputFileFlag,
    "Filepath of output maze PNG file.", "PATHNAME", NULL);

  printf("Optional arguments:\n");
  PrintRangedFlag(kMazeWidthFlag, "Number of cells per maze row.", "N",
    kMazeWidthMin, kMazeWidthMax, kMazeWidthDefault);
  PrintRangedFlag(kMazeHeightFlag, "Number of cells per maze column.", "M",
    kMazeHeightMin, kMazeHeightMax, kMazeHeightDefault);
  PrintFlag(kSeedFlag,
    "Value used to be seed the random number generator used.  "
    "Can be a positive integer or \"time\" to use system time.",
    "SEED", kSeedDefaultName);
  PrintRangedFlag(kCellWidthFlag, "Square side-length of maze cell in pixels.",
    "N", kCellWidthMin, kCellWidthMax, kCellWidthDefault);
  PrintRangedFlag(kWallWidthFlag, "Thinkness maze walls in pixels.", "N",
    0, kWallWidthMax, kWallWidthDefault);
  PrintRangedFlag(kBorderWidthFlag, "Thinkness maze border in pixels.", "N",
    0, kBorderWidthMax, kBorderWidthDefault);
  PrintFlag(kCellColorFlag,
    "Color of maze cell. Ignored with any other cell color setting.  "
    "See below for known colors.", kColor, kCellColorDefaultName);
  PrintFlag(kCellColorMetricFlag,
    "Metric for determining the color of each maze cell.  "
    "Only valid if cell color mode is set.  "
    "See below for known color metrics.",
    kColorMetric, kCellColorMetricDefaultName);
  PrintFlag(kCellColorModeFlag,
    "Type of color selection algorithm used to color a maze cell.  "
    "Only valid if cell color metric is set.  "
    "See below for known color modes.",
    kColorMode, kCellColorModeDefaultName);
  PrintFlag(kConnColorFlag,
    "Color of connection between maze cells.  "
    "Ignored with any cell coloring mode set, or if maze wall thinkness is 0.  "
    "See below for known colors", kColor, kConnColorDefaultName);
  PrintFlag(kConnColorMethodFlag,
    "Method used to determine the color between connected maze cells.  "
    "Useful when using a cell color mode.  "
    "Ignored if maze wall thinkness is 0.",
    kColorMethod, kConnColorMethodDefaultName);
  PrintFlag(kWallColorFlag,
    "Color of maze walls.  "
    "Ignored if maze wall thinkness is 0.  "
    "See below for known colors.", kColor, kWallColorDefaultName);
  PrintFlag(kBorderColorFlag,
    "Color of maze border.  "
    "Ignored if maze border thinkness is 0.  "
    "See below for known colors.",
    kColor, kBorderColorDefaultName);
  PrintFlag(kDrawPathFlag,
    "Draws a solution path from the top right corner to the bottom left.",
    NULL, NULL);
  PrintFlag(kPathColorFlag,
    "Color of the solution path that is drawn.  "
    "Ignored if path drawing is not enabled.", kColor, kPathColorDefaultName);

  printf("Developer arguments:\n");
  PrintFlag(kDebugModeFlag,
    "Enables some additional logs and internal checks.  "
    "Intended to be used by program developer, not a user.", NULL, NULL);

  printf("Known values:\n");

  for (i = 0; i < kKnownColorsCount; i++)
  {
    buf[i] = kKnownColors[i].color_name;
  }
  PrintKnownValues(kColor, buf, kKnownColorsCount);

  for (i = 0; i < kKnownColorMetricCount; i++)
  {
    buf[i] = kKnownColorMetrics[i].color_metric_name;
  }
  PrintKnownValues(kColorMetric, buf, kKnownColorMetricCount);

  for (i = 0; i < kKnownColorModesCount; i++)
  {
    buf[i] = kKnownColorModes[i].color_mode_name;
  }
  PrintKnownValues(kColorMode, buf, kKnownColorModesCount);

  for (i = 0; i < kKnownColorMethodsCount; i++)
  {
    buf[i] = kKnownColorMethods[i].color_method_name;
  }
  PrintKnownValues(kColorMode, buf, kKnownColorMethodsCount);
  printf("\nCopyright (c) 2019 Alex Dale\n");
  printf("This software is distributed under the MIT License\n");
}

static void PrintFlag(
  char const *flag, char const *description,
  char const *rep, char const *deflt)
{
  size_t indent;
  char buf[STR_BUF_SZ];
  if (!flag) return;
  if (rep)
  {
    snprintf(buf, STR_BUF_SZ, "%s %s", flag, rep);
    indent = printf("  %-16s  ", buf);
  }
  else
  {
    indent = printf("  %-16s  ", flag);
  }
  if (description)
  {
    indent = PrintDescriptionColumn(description, indent);
  }
  if (deflt)
  {
    snprintf(buf, STR_BUF_SZ, " (default: %s)", deflt);
    PrintDescriptionColumn(buf, indent);
  }
  printf("\n");
}

static void PrintRangedFlag(char const *flag, char *description, char const *rep, size_t min, size_t max, size_t deflt)
{
  size_t indent;
  char buf[STR_BUF_SZ];
  if (!flag || !rep || max < min) return;
  snprintf(buf, STR_BUF_SZ, "%s %s", flag, rep);
  indent = printf("  %-16s  ", buf);
  if (description)
  {
    indent = PrintDescriptionColumn(description, indent);
  }
  if (min > 0)
  {
    snprintf(buf, STR_BUF_SZ, " (range: %lu <= %s <= %lu, default: %lu)", min, rep, max, deflt);
  }
  else
  {
    snprintf(buf, STR_BUF_SZ, " (range: %s <= %lu, default: %lu)", rep, max, deflt);
  }
  PrintDescriptionColumn(buf, indent);
  printf("\n");
}

static void PrintKnownValues(char const *value_name, kstring_t *values, size_t values_count)
{
  size_t indent, i;
  char buf[STR_BUF_SZ];
  if (!value_name || !values || values_count == 0) return;
  indent = printf("  %-16s  ", value_name);
  buf[0] = 0;
  for (i = 0; i < values_count; i++)
  {
    if (i > 0) AppendString(buf, ", ", STR_BUF_SZ);
    AppendString(buf, values[i], STR_BUF_SZ);
  }
  PrintDescriptionColumn(buf, indent);
  printf("\n");
}

static size_t PrintDescriptionColumn(char const *description, size_t indent)
{
  /* Pointer to current description character, and to the begining of the
   * next word. */
  char const *dptr, *nptr;
  size_t col;
  col = indent;
  dptr = description;
  while (*dptr)
  {
    /* Apply indentation if needed */
    if (col < kFlagIndentSize)
    {
      for (; col < kFlagIndentSize; col++)  putchar(' ');
    }
    /* If dptr is currently at white space, check to see if the white+space
     * plus the length of the next non-white space will fit on the current
     * line. */
    if (*dptr == ' ')
    {
      nptr = dptr + 1;
      while (*nptr == ' ') nptr++;
      /* Check if the white space alone is too long. */
      if (col + (nptr - dptr) > kMaxTermWidth)
      {
        putchar('\n');
        col = 0;
        while (*dptr == ' ') dptr++;
        continue;
      }
      /* Check if white space + next word is too long. */
      while (*nptr && *nptr != ' ') nptr++;
      if (col + (nptr - dptr) > kMaxTermWidth)
      {
        putchar('\n');
        col = 0;
        /* Skip white space. */
        while (*dptr == ' ') dptr++;
        continue;
      }
      /* Insert white space. */
      while (*dptr == ' ')
      {
        putchar(*dptr++);
        col++;
      }
    }
    /* Check if current word will fit on the line. */
    nptr = dptr + 1;
    while (*nptr && *nptr != ' ') nptr++;
    /* Check if the signle word is too long, if so, split it. */
    if (((size_t) (nptr - dptr)) > kMaxTermWidth)
    {
      while (*dptr && *dptr != ' ' && (col + 1) < kMaxTermWidth)
      {
        putchar(*dptr++);
        col++;
      }
      putchar('-');
      putchar('\n');
      col = 0;
      continue;
    }
    /* Check if adding the word will over flow. */
    if (col + (nptr - dptr) > kMaxTermWidth)
    {
      putchar('\n');
      col = 0;
      continue;
    }
    /* Print word */
    while (*dptr && *dptr != ' ')
    {
      putchar(*dptr++);
      col++;
    }
  } /* while (*dptr) */
  return col;
}

void MazartDefaultParameters(mazart_config_t *config)
{
  if (!config) return;
  memset(config, 0, sizeof(mazart_config_t));
  config->maze_width = kMazeWidthDefault;
  config->maze_height = kMazeHeightDefault;
  config->seed = time(NULL);
  config->cell_width = kCellWidthDefault;
  config->wall_width = kWallWidthDefault;
  config->border_width = kBorderWidthDefault;
  config->cell_color = kCellColorDefault;
  config->cell_color_metric = kCellColorMetricDefault;
  config->cell_color_mode = kCellColorModeDefault;
  config->conn_color = kConnColorDefault;
  config->conn_color_method = kConnColorMethodDefault;
  config->wall_color = kWallColorDefault;
  config->border_color = kBorderColorDefault;
  config->path_color = kPathColorDefault;
}

void PrintMazartConfit(mazart_config_t *config)
{
  if (!config) return;
  puts("{");
  printf("  \"maze_width\": %lu,\n", config->maze_width);
  printf("  \"maze_height\": %lu,\n", config->maze_height);
  printf("  \"seed\": %lu,\n", config->seed);
  printf("  \"cell_width\": %lu,\n", config->cell_width);
  printf("  \"wall_width\": %lu,\n", config->wall_width);
  printf("  \"border_width\": %lu,\n", config->border_width);
  if (config->cell_color != CLR_OTHER && config->cell_color != CLR_NONE)
  {
    printf("  \"cell_color\": \"%s\",\n", ColorToString(config->cell_color));
  }
  if (config->cell_color_metric != CLR_MTRC_NONE)
  {
    printf("  \"cell_color_metric\": \"%s\",\n",
      ColorMetricToString(config->cell_color_metric));
  }
  if (config->cell_color_mode != CLR_MODE_NONE)
  {
    printf("  \"cell_color_mode\": \"%s\",\n",
      ColorModeToString(config->cell_color_mode));
  }
  if (config->conn_color != CLR_OTHER && config->conn_color != CLR_NONE)
  {
    printf("  \"conn_color\": \"%s\",\n", ColorToString(config->conn_color));
  }
  if (config->conn_color_method != CLR_MTHD_NONE)
  {
    printf("  \"conn_color_method\": \"%s\",\n",
      ColorMethodToString(config->conn_color_method));
  }
  if (config->wall_color != CLR_OTHER && config->wall_color != CLR_NONE)
  {
    printf("  \"wall_color\": \"%s\",\n", ColorToString(config->wall_color));
  }
  if (config->border_color != CLR_OTHER && config->border_color != CLR_NONE)
  {
    printf("  \"border_color\": \"%s\",\n", ColorToString(config->border_color));
  }
  if (config->path_color != CLR_OTHER && config->path_color != CLR_NONE && config->draw_path)
  {
    printf("  \"path_color\": \"%s\",\n", ColorToString(config->path_color));
  }
  if (config->output_file)
  {
    printf("  \"output_file\": \"%s\"\n", config->output_file);
  }
  else
  {
    printf("  \"output_file\": null\n");
  }
  puts("}");
}

#define GET_INTEGER_MAX_MIN(arg, value, name, max, min) ({ \
  size_t v; \
  if (!value) { \
    fprintf(stderr, "Error: Expected integer after %s\n", arg); \
    return false; \
  } \
  if (!IsInteger(value)) { \
    fprintf(stderr, "Error: Expected integer after %s, got %s\n", arg, value); \
    return false; \
  } \
  v = ParseInteger(value); \
  if (v > max) { \
    fprintf(stderr, "Error: Max value for %s is %lu, got %lu\n", arg, max, v); \
    return false; \
  } \
  if (v < min) { \
    fprintf(stderr, "Error: Min value for %s is %lu, got %lu\n", arg, min, v); \
    return false; \
  } \
  v; \
})
#define GET_INTEGER_MAX(arg, value, name, max) GET_INTEGER_MAX_MIN(arg, value, name, max, kZero)
#define GET_INTEGER(arg, value, name) GET_INTEGER_MAX(arg, value, name, SIZE_MAX)

#define GET_COLOR(arg, value, name) ({ \
  mazart_color_t c; \
  if (!value) { \
    fprintf(stderr, "Error: Expected color after %s\n", arg); \
    return false; \
  } \
  if (!IsColor(value)) { \
    fprintf(stderr, \
      "Error: Expected color after %s, got %s; " \
      "see --help for available colors\n", arg, value); \
    return false; \
  } \
  c = ParseColor(value); \
  c; \
})

#define GET_COLOR_METRIC(arg, value, name) ({ \
  mazart_color_metric_t c; \
  if (!value) { \
    fprintf(stderr, "Error: Expected color metric after %s\n", arg); \
    return false; \
  } \
  if (!IsColorMetric(value)) { \
    fprintf(stderr, \
      "Error: Expected color metric after %s, got %s; " \
      "see --help for available color metrics\n", arg, value); \
    return false; \
  } \
  c = ParseColorMetric(value); \
  c; \
})

#define GET_COLOR_MODE(arg, value, name) ({ \
  mazart_color_mode_t c; \
  if (!value) { \
    fprintf(stderr, "Error: Expected color mode after %s\n", arg); \
    return false; \
  } \
  if (!IsColorMode(value)) { \
    fprintf(stderr, \
      "Error: Expected color mode after %s, got %s; " \
      "see --help for available color modes\n", arg, value); \
    return false; \
  } \
  c = ParseColorMode(value); \
  c; \
})

#define GET_COLOR_METHOD(arg, value, name) ({ \
  mazart_color_method_t c; \
  if (!value) { \
    fprintf(stderr, "Error: Expected color method after %s\n", arg); \
    return false; \
  } \
  if (!IsColorMethod(value)) { \
    fprintf(stderr, \
      "Error: Expected color method after %s, got %s; " \
      "see --help for available color methods\n", arg, value); \
    return false; \
  } \
  c = ParseConnColorMethod(value); \
  c; \
})

#define VAL_CONTINUE i++; continue;

bool_t ParseMazartParameters(char const * const *args, size_t arg_count, mazart_config_t *config)
{
  size_t i;
  char const *prog;
  if (!args || arg_count == 0 || !config) return false;
  prog = args[0];
  MazartDefaultParameters(config);
  for (i = 1; i < arg_count; i++)
  {
    if (StringsEqual(args[i], "--help") || StringsEqual(args[i], "-h"))
    {
      PrintUsage(args[0]);
      return false;
    }
  }
  for (i = 1; i < arg_count; i++)
  {
    char const *arg, *value;
    arg = args[i];
    value = ((i + 1) < arg_count) ? args[i+1] : NULL;

    if (StringsEqual(arg, kDebugModeFlag))
    {
      config->debug_mode = true;
      continue;
    }
    if (StringsEqual(arg, kMazeWidthFlag))
    {
      config->maze_width =
        GET_INTEGER_MAX_MIN(arg, value, kMazeWidthFlag, kMazeWidthMax, kMazeWidthMin);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kMazeHeightFlag))
    {
      config->maze_height =
        GET_INTEGER_MAX_MIN(arg, value, kMazeHeightFlag, kMazeHeightMax, kMazeHeightMin);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kSeedFlag))
    {
      /* Time is alread the default. */
      if (StringsEqual(value, "time")) { VAL_CONTINUE };
      config->seed = GET_INTEGER(arg, value, kSeedFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kCellWidthFlag))
    {
      config->cell_width =
        GET_INTEGER_MAX_MIN(arg, value, kCellWidthFlag, kCellWidthMax, kCellWidthMin);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kWallWidthFlag))
    {
      config->wall_width =
        GET_INTEGER_MAX(arg, value, kWallWidthFlag, kWallWidthMax);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kBorderWidthFlag))
    {
      config->border_width =
        GET_INTEGER_MAX(arg, value, kBorderWidthFlag, kBorderWidthMax);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kCellColorFlag))
    {
      config->cell_color =
        GET_COLOR(arg, value, kCellColorFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kCellColorMetricFlag))
    {
      config->cell_color_metric =
        GET_COLOR_METRIC(arg, value, kCellColorMetricFlag);
      VAL_CONTINUE
    }
    if (StringsEqual(arg, kCellColorModeFlag))
    {
      config->cell_color_mode =
        GET_COLOR_MODE(arg, value, kCellColorModeFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kConnColorFlag))
    {
      config->conn_color =
        GET_COLOR(arg, value, kConnColorFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kConnColorMethodFlag))
    {
      config->conn_color_method =
        GET_COLOR_METHOD(arg, value, kConnColorMethodFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kWallColorFlag))
    {
      config->wall_color =
        GET_COLOR(arg, value, kWallColorFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kBorderColorFlag))
    {
      config->border_color =
        GET_COLOR(arg, value, kBorderColorFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kDrawPathFlag))
    {
      config->draw_path = true;
      continue;
    }
    if (StringsEqual(arg, kPathColorFlag))
    {
      config->path_color =
        GET_COLOR(arg, value, kPathColorFlag);
      VAL_CONTINUE;
    }
    if (StringsEqual(arg, kOutputFileFlag))
    {
      if (!IsFileName(value)) return false;
      config->output_file = ParseFileName(value);
      VAL_CONTINUE;
    }
    fprintf(stderr, "Error: unknown argument %s\n", arg);
    PrintUsage(prog);
    return false;
  }
  /* Apply simple rules */
  if (config->border_color == CLR_OTHER)
  {
    config->border_color = config->wall_color;
  }
  if (config->cell_color_mode != CLR_MODE_NONE && config->cell_color_metric == CLR_MTRC_NONE)
  {
    fprintf(stderr, "Error: Color metric cannot be none if color mode is set\n");
    return false;
  }
  if (config->cell_color_mode == CLR_MODE_NONE && config->cell_color_metric != CLR_MTRC_NONE)
  {
    fprintf(stderr, "Error: Color mode cannot be none if color metric is set\n");
    return false;
  }
  if (!config->output_file)
  {
    fprintf(stderr, "Error: %s is required\n", kOutputFileFlag);
    return false;
  }
  return true;
}

bool_t MazartColorToColor(mazart_color_t ma_color, rgb_t *color)
{
  switch (ma_color)
  {
    case CLR_WHITE:
      *color = (rgb_t) { .red = 255, .green = 255, .blue = 255 };
      return true;
    case CLR_LIGHT_GREY:
      *color = (rgb_t) { .red = 225, .green = 225, .blue = 225 };
      return true;
    case CLR_GREY:
      *color = (rgb_t) { .red = 127, .green = 127, .blue = 127 };
      return true;
    case CLR_BLACK:
      *color = (rgb_t) { .red = 0, .green = 0, .blue = 0 };
      return true;
    case CLR_BLUE:
      *color = (rgb_t) { .red = 54, .green = 54, .blue = 255 };
      return true;
    case CLR_TEAL:
      *color = (rgb_t) { .red = 54, .green = 208, .blue = 208 };
      return true;
    case CLR_GREEN:
      *color = (rgb_t) { .red = 54, .green = 255, .blue = 54 };
      return true;
    case CLR_YELLOW:
      *color = (rgb_t) { .red = 255, .green = 255, .blue = 54 };
      return true;
    case CLR_ORANGE:
      *color = (rgb_t) { .red = 255, .green = 54, .blue = 0 };
      return true;
    case CLR_RED:
      *color = (rgb_t) { .red = 255, .green = 0, .blue = 0 };
      return true;
    case CLR_PURPLE:
      *color = (rgb_t) { .red = 208, .green = 0, .blue = 208 };
      return true;
    case CLR_NONE:
    case CLR_OTHER:
    default:
      return false;
  }
}
