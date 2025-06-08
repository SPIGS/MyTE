#pragma once
#include <stdbool.h>
#include "putils/defines.h"


char *readFile(const char *file_path);
void writeFile(const char *path, const char *bytes);
i32 checkPath(const char *path);
char *getFileNameFromPath(const char *file_path);
