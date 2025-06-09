#include "file.h"
#include "unistd.h"
#include <stddef.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <putils/log.h>

// Define macros for platform differences
#if defined(__linux__)
    #define IFFILE __S_IFREG
    #define IFDIR __S_IFDIR
#elif defined(__APPLE__)
    #define IFFILE S_IFREG
    #define IFDIR S_IFDIR
#endif

// TODO: handle errors more gracefully
char *readFile(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buf, 1, size, f);
    buf[bytes_read] = '\0';


    fclose(f);
    return buf;
}

void writeFile(const char *path, const char *bytes) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        LOG_ERROR("Could not open file with write access: \'%s\'", path);
        return;
    }

    if (fputs(bytes, f) == EOF) {
        LOG_ERROR("Couldn't write to file: \'%s\'", path);
        fclose(f);
        return;
    }
    fclose(f);
}

// Returns 0 if it is a file, 1 if it is a directory and -1 if there was an error (or doesn't exist)
i32 checkPath(const char *path) {
    struct stat s;
    if (stat(path, &s) == 0) {
        if (s.st_mode & IFFILE) {
                return 0; // It's a file
        } else if (s.st_mode & IFDIR) {
            return 1; // It's a directory
        }
    }
    return -1; // Error (e.g. file not found)
}

