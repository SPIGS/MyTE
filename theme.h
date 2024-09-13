#pragma once
#include "util.h"
#include "toml.h"

#define LOAD_TOML_INT(table, key, var) \
    toml_datum_t var = toml_int_in(table, key); \
    if (!var.ok) { \
        LOG_ERROR("cannot read ", key); \
        return; \
    }

#define LOAD_TOML_STR(table, key, var) \
    toml_datum_t var = toml_string_in(table, key); \
    if (!var.ok) { \
        LOG_ERROR("cannot read ", key); \
        return; \
    }

#define LOAD_TOML_BOOL(table, key, var) \
    toml_datum_t var = toml_bool_in(table, key); \
    if (!var.ok) { \
        LOG_ERROR("cannot read ", key); \
        return; \
    }

#define LOAD_TOML_ARRAY(table, key, var) \
    toml_array_t *var = toml_array_in(table, key); \
    if (!var) { \
        LOG_ERROR("cannot read ", key); \
        return; \
    }


typedef struct {
    i32 keyword_color;
    i32 symbol_color;
    i32 string_literal_double_color;
    i32 string_literal_single_color;
    i32 number_color;
    i32 built_in_type_color;
    i32 type_color;
    i32 function_name_color;
    i32 foreground_color;
    i32 background_color;
    i32 comment_single_color;
    i32 comment_multi_color;
} ColorTheme;

ColorTheme colorThemeInit();
void colorThemeLoad(ColorTheme *theme, const char *path);

typedef struct {
    char *theme_path;
    i32 tab_stop;
} Config;

Config configInit();
void configDestroy(Config *config);
void loadConfigFromFile(Config *config, const char* path);