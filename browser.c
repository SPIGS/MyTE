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
        if (is_dir) {
            strcat(dst, "/"); // Append "/" if it's a directory
        }
    }
    return dst;
}

void fileBrowserInit(FileBrowser *fb, vec2 selection_screen_pos) {
    fb->items = NULL;
    fb->num_paths = 0;
    fb->selection = 0;

    fb->sel_screen_pos = selection_screen_pos;
    fb->sel_prev_screen_pos = selection_screen_pos;
    fb->sel_target_screen_pos = selection_screen_pos;
    fb->sel_size = vec2_init(0.0f, 0.0f);
    fb->sel_prev_size = vec2_init(0.0f, 0.0f);
    fb->sel_target_size = vec2_init(0.0f, 0.0f);
    fb->anim_time = 0.0;
}

void fileBrowserDestroy(FileBrowser *fb) {
    if (fb->items) {
        for (size_t i = 0; i < fb->num_paths; i++) {
            free(fb->items[i].full_path);
            free(fb->items[i].name_ext);
        }
    }       
}

// Function to list paths (../ first) and store in path_list, appending "/" to directories
void getPaths(FileBrowser *fb, const char *path) {
    DIR *dir;
    struct dirent *entry;
    size_t count = 0;
    size_t allocated_size = 10;
    BrowserItem* items = NULL;
    char full_path[MAX_PATH_LEN];

    // Open the directory
    if (!(dir = opendir(path))) {
        LOG_ERROR("Couldn't open directory in file browser ", "");
        return;
    }

    // Allocate memory for the path list
    items = malloc(sizeof(BrowserItem) * allocated_size); // Initial size for paths
    if (!items) {
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
        snprintf(full_path, MAX_PATH_LEN, "%s/%s", path, entry->d_name);

        // Determine if it's a directory
        int is_dir = is_directory(full_path);

        // Add to the list, appending "/" if it's a directory
        // Brwoser ITem

        // Allocate and store the Item
        items[count].full_path = allocate_and_copy(full_path, is_dir);  // Store the full path
        items[count].name_ext = allocate_and_copy(entry->d_name, is_dir);  // Store the name and extension
        items[count].is_dir = is_dir;  // Store if it's a directory
        count++;

        // Reallocate memory if needed
        if (count >= allocated_size) {
            allocated_size += 10;
            items = realloc(items, sizeof(*items) * allocated_size);
            if (!items) {
                LOG_ERROR("Couldn't reallocate memory for browser's path list.", "");
                closedir(dir);
                return;
            }
        }
    }

    // Close the directory
    closedir(dir);

    // sort the paths
    qsort(items, count, sizeof(items[0]), compareItems);

    fb->items = items;
    fb->num_paths = count;
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

const char *getSelection(FileBrowser *fb) {
    return fb->items[fb->selection].full_path;
}