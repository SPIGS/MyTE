#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "util.h"

typedef struct {
    char *full_path;
    char *name_ext;
    bool is_dir;
} BrowserItem;

typedef struct {
    BrowserItem *items;
    size_t num_paths;
    size_t selection;
    char* cur_dir;

    // Render info
    vec2 scroll_pos;

    vec2 sel_screen_pos;
    vec2 sel_prev_screen_pos;
    vec2 sel_target_screen_pos;
    vec2 sel_size;
    vec2 sel_prev_size;
    vec2 sel_target_size;
    f32 anim_time;
} FileBrowser;

void fileBrowserInit(FileBrowser *fb, vec2 selection_screen_pos, const char * cur_dir);
void fileBrowserDestroy(FileBrowser *fb);

// Retrieve the list of files/folders from the browser's root directory
void getPaths(FileBrowser *fb);

// Enter a directory
void enterDirectory(FileBrowser *fb, const char *dir_name);

// Go up directory level
void goUpDirectoryLevel(FileBrowser *fb);

// Changes the browser's root directory to an arbitrary value
void changeRootDirectory(FileBrowser *fb, const char *new_root);

void incrementSelection(FileBrowser *fb);
void decrementSelection(FileBrowser *fb);
BrowserItem getSelection(FileBrowser *fb);