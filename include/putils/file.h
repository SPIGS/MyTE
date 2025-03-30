#pragma once
#include <stdbool.h>


char *readFile(const char *file_path);
bool filePathExists(const char *path);
bool isDir(const char *path);
bool isFile(const char *path);
/*void fileRename(const char *path, const char *new_name);*/

