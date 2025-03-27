#include "file.h"
#include "unistd.h"
#include <sys/stat.h>

bool filePathExists(const char *path) {
  return access(path, F_OK) == 0;
}

bool isDir(const char *path) {
  struct stat s;
  bool result = false;
  if (stat(path, &s) == 0) {
    if (s.st_mode & S_ISDIR(s.st_mode)) {
      result = true;
    }
  }
  return result;
}

bool isFile(const char *path) {
  struct stat s;
  bool result = false;
  if (stat(path, &s) == 0) {
    if (s.st_mode & S_ISREG(s.st_mode)) {
      result = true;
    }
  }
  return result;
}

/*void fileRename(const char *path, const char* new_name) {*/
/**/
/*}*/
