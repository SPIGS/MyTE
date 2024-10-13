#include <stdlib.h>
#include "browser.h"
#include "util.h"

#define MAX_PATH_LEN 1024

// Function to determine if the given path is a directory
static bool is_directory(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static int compareItems (const void *a, const void *b) {
    BrowserItem *item_a = (BrowserItem *)a;
    BrowserItem *item_b = (BrowserItem *)b;
    return strcmp( item_a->name_ext, item_b->name_ext);
}

// Function to safely allocate and copy strings with optional "/" for directories
static char *allocate_and_copy(const char *src, int is_dir) {
    size_t len = strlen(src) + (is_dir ? 2 : 1); // Extra space for '/' if it's a directory
    char *dst = malloc(len);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}

void fileBrowserInit(FileBrowser *fb, vec2 selection_screen_pos, const char *cur_dir) {
    fb->items = NULL;
    fb->num_paths = 0;
    fb->paths_capacity = 10;
    fb->selection = 0;
    fb->cur_dir = malloc(strlen(cur_dir) + 1);
    strcpy(fb->cur_dir, cur_dir);

    fb->scroll_pos = vec2_init(0,0);
    fb->sel_screen_pos = selection_screen_pos;
    fb->sel_prev_screen_pos = selection_screen_pos;
    fb->sel_target_screen_pos = selection_screen_pos;
    fb->sel_size = vec2_init(0.0f, 0.0f);
    fb->sel_prev_size = vec2_init(0.0f, 0.0f);
    fb->sel_target_size = vec2_init(0.0f, 0.0f);
    fb->anim_time = 0.0;
}

void browserItemInit(BrowserItem *item) {
    item->full_path = NULL;
    item->is_dir = false;
    item->name_ext = NULL;
}

void browserItemDestroy (BrowserItem *item) {
    if (item->full_path) {
        free(item->full_path);
        item->full_path = NULL;
    }
    
    if (item->name_ext) {
        free(item->name_ext);
        item->name_ext = NULL;
    }
}

void fileBrowserDestroy(FileBrowser *fb) {
    if (fb->items) {
        for (size_t i = 0; i < fb->num_paths; i++) {
            browserItemDestroy(&fb->items[i]);
        }
        free(fb->items);
    }

    if (fb->cur_dir) {
        free(fb->cur_dir);
    }
}

// Function to list paths (../ first) and store in path_list, appending "/" to directories
void getPaths(FileBrowser *fb) {
    DIR *dir;
    struct dirent *entry;
    size_t inital_capacity = 10;
    char full_path[MAX_PATH_LEN];

    // Clear paths list
    if (fb->items) {
        for (size_t i = 0; i < fb->num_paths; i++) {
            browserItemDestroy(&fb->items[i]);
        }
        free(fb->items);
        fb->num_paths = 0;
        fb->paths_capacity = inital_capacity;
    }

    // Open the directory
    if (!(dir = opendir(fb->cur_dir))) {
        LOG_ERROR("Couldn't open directory in file browser ", "");
        return;
    }

    // Allocate memory for the path list
    fb->items = malloc(inital_capacity * sizeof(BrowserItem)); // Initial size for paths
    if (!fb->items) {
        LOG_ERROR("Couldn't allocate memory for browser's file path list.", "");
        closedir(dir);
        return;
    }

    // Read directory contents
    while ((entry = readdir(dir)) != NULL) {
        // Skip the "." entry
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }

        // Build the full path for stat checking
        snprintf(full_path, MAX_PATH_LEN, "%s/%s", fb->cur_dir, entry->d_name);

        // Determine if it's a directory
        int is_dir = is_directory(full_path);

        // Allocate and store the Item
        BrowserItem item;
        browserItemInit(&item);
        item.full_path = allocate_and_copy(full_path, is_dir); // Store the full path
        item.name_ext = allocate_and_copy(entry->d_name, is_dir); // Store the name and extension
        item.is_dir = is_dir; // Store if it's a directory

        // Reallocate memory if needed
        if (fb->num_paths >= fb->paths_capacity) {
            fb->paths_capacity *= 2;
            BrowserItem *new_items = realloc(fb->items, fb->paths_capacity * sizeof(BrowserItem));
            if (!new_items) {
                LOG_ERROR("Couldn't reallocate memory for browser's path list.", "");
                closedir(dir);
                return;
            }
            fb->items = new_items;
        }

        fb->items[fb->num_paths] = item;
        fb->num_paths++;
    }

    // Close the directory
    closedir(dir);

    // sort the paths
    qsort(fb->items, fb->num_paths, sizeof(BrowserItem), compareItems);
}

void enterDirectory(FileBrowser *fb, const char *dir_name) {
    // Append name of dir to cur_dir
    fb->cur_dir = realloc(fb->cur_dir, strlen(fb->cur_dir) + strlen(dir_name) + 2);
    strcat(fb->cur_dir, "/");
    strcat(fb->cur_dir, dir_name);
}

void goUpDirectoryLevel(FileBrowser *fb) {
    const char *last_slash = strrchr(fb->cur_dir, '/');

    if (last_slash == NULL) {
        return;
    }
    size_t len = last_slash - fb->cur_dir;

    char * new_dir = (char*)malloc((len + 1));
    strncpy(new_dir, fb->cur_dir, len);
    new_dir[len] = '\0';
    free(fb->cur_dir);
    fb->cur_dir = new_dir;
}

void changeRootDirectory(FileBrowser *fb, const char *new_root) {
    if (strlen(new_root) > strlen(fb->cur_dir)) {
        fb->cur_dir = realloc(fb->cur_dir, strlen(new_root) + 1);
        strcpy(fb->cur_dir, new_root);
    } 
}

void incrementSelection(FileBrowser *fb) {
    fb->selection ++;
    if (fb->selection >= fb->num_paths) {
        fb->selection = fb->num_paths - 1;
    }
    fb->anim_time = 0.0f;
}

void decrementSelection(FileBrowser *fb) {
    if (fb->selection == 0)
        return;
    fb->selection --;
    fb->anim_time = 0.0f;
}

BrowserItem getSelection(FileBrowser *fb) {
    return fb->items[fb->selection];
}