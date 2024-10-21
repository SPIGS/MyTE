#pragma once
#include "util.h"
#include "toml.h"
#include "keys.h"

#define LOAD_TOML_INT(table, var) \
    toml_datum_t var = toml_int_in(table, #var); \
    if (!var.ok) { \
        LOG_WARN("cannot read TOML int \'%s\'", #var); \
    }

#define LOAD_TOML_STR(table, var) \
    toml_datum_t var = toml_string_in(table, #var); \
    if (!var.ok) { \
        LOG_WARN("cannot read TOML string \'%s\'", #var); \
    }

#define LOAD_TOML_BOOL(table, var) \
    toml_datum_t var = toml_bool_in(table, #var); \
    if (!var.ok) { \
        LOG_WARN("cannot read TOML bool \'%s\'", #var); \
    }

#define LOAD_TOML_STR_ARRAY(table, key, array, array_count, var, count) \
    toml_array_t *array = toml_array_in(table, key); \
    size_t count = 0; \
    char **var = NULL; \
    if (!array) { \
        LOG_WARN("cannot read TOML string array \'%s\'", key); \
    } else { \
        size_t array_count = toml_array_nelem(array); \
        var = malloc(array_count * sizeof(char *)); \
        count = 0; \
        for (size_t i = 0; i < array_count; i++) { \
            char *str = toml_string_at(array, i).u.s; \
            var[count] = malloc(strlen(str) + 1); \
            strcpy(var[count], str); \
            free(str); \
            count++; \
        } \
    }
    
#define LOAD_TOML_DOUBLE(table, var) \
    toml_datum_t var = toml_double_in(table, #var); \
    if (!var.ok) { \
        LOG_WARN("cannot read TOML double \'%s\'", #var); \
    }

typedef struct {
    // general
    Color foreground;
    Color background;
    Color current_line;
    Color user_selection;
    Color gutter_foreground;

    // syntax
    Color keyword;
    Color secondary_keyword;
    Color built_in_type;
    Color type;
    Color function_name;
    Color symbol;
    Color double_quote_string;
    Color single_quote_string;
    Color number;
    Color single_line_comment;
    Color multiline_comment;
} ColorTheme;

ColorTheme colorThemeInit();
void colorThemeLoad(ColorTheme *theme, const char *path);

typedef struct {
    int key;
    int mods;
    char *command_name;
} CommandConfig;

typedef struct {
    // General
    char *font_path;
    char *theme_path;
    i32 font_size;
    bool show_fps;

    // Editor
    i32 tab_stop;
    f64 cursor_speed;
    i32 scroll_speed;
    i32 scroll_stop_top;
    i32 scroll_stop_bottom;

    CommandConfig commandConfigs[100];
    size_t numCommandConfigs;
} Config;

Config configInit();
void configDestroy(Config *config);
void loadConfigFromFile(Config *config, const char* path);