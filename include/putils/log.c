#include "log.h"
#include <stdio.h>
#include <stdarg.h>

static const char *level_strings[] = {
  "DEBUG", "INFO", "WARN", "ERROR",
};

static const char *level_colors[] = {
  "\033[36m", "\033[39m", "\033[33m", "\033[31m",
};

void log_log(LogLevel level, const char *file, int line, const char *fmt, ...) {
    va_list args;

    fprintf(stderr, "%s%s %s:%d %s: ", level_colors[level], level_strings[level], file, line, level_colors[1]);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}
