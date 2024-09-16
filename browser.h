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

    // Render info
    vec2 sel_screen_pos;
    vec2 sel_prev_screen_pos;
    vec2 sel_target_screen_pos;
    vec2 sel_size;
    vec2 sel_prev_size;
    vec2 sel_target_size;
    f32 anim_time;
} FileBrowser;

void fileBrowserInit(FileBrowser *fb, vec2 selection_screen_pos);
void fileBrowserDestroy(FileBrowser *fb);
void getPaths(FileBrowser *fb, const char *path);
void incrementSelection(FileBrowser *fb);
void decrementSelection(FileBrowser *fb);
const char *getSelection(FileBrowser *fb);