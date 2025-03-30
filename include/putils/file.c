#include "file.h"
#include "unistd.h"
#include <stddef.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <putils/log.h>

// TODO: handle errors more gracefully
char *readFile(const char *file_path) {
  FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        LOG_ERROR("Could not open file \'%s\'", file_path);
        exit(1);
    }
    size_t capacity = 1024;
    char *read_buf = malloc(capacity);
    if (read_buf == NULL) {
        LOG_ERROR("Could allocate space for buffer to read file.", "");
        fclose(f);
        exit(1);
    }
    
    size_t nread;
    size_t size = 0;
    char *temp = NULL;
    while ((nread = fread(read_buf + size, 1, 1024, f)) > 0) {
        size += nread;
        if (size > capacity) {
            capacity *= 2;
            temp = realloc(read_buf, capacity);
            read_buf = temp;
        }
    }
    read_buf[size] = '\0';
    fclose(f);
    return read_buf;

}

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
