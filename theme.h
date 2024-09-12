#pragma once
#include "util.h"
#include "toml.h"

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