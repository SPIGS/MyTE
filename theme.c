#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "theme.h"

static void error(const char* msg, const char* msg1)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
}


ColorTheme colorThemeInit() {
    // Colors - the default theme is Space Duck (https://github.com/pineapplegiant/spaceduck)
    ColorTheme theme;
    theme.keyword_color = 0x7A5CCCFF;          
    theme.symbol_color =  0xCE6F8FFF;
    theme.string_literal_double_color = 0x00A3CCFF;
    theme.string_literal_single_color = 0xF2CE00FF;
    theme.number_color = 0xF2CE00FF;           
    theme.built_in_type_color = 0x5CCC96FF;
    theme.type_color = 0xF2CE00FF;          
    theme.function_name_color = 0x5CCC96FF;  
    theme.foreground_color = 0xFFFFFFFF;
    theme.background_color = 0x0F111BFF;      
    theme.comment_single_color = 0x686f9aFF;    
    theme.comment_multi_color = 0x686f9aFF;
    return theme;
}

void colorThemeLoad(ColorTheme *theme, const char *path) {
    FILE *fp;
    char errbuf[200];

    fp = fopen(path, "r");
    
    toml_table_t *color_conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!color_conf) {
        error("cannot open color theme file - ", strerror(errno));
        return;
    }

    toml_table_t* color_table = toml_table_in(color_conf, "colors");
    if (!color_conf) {
        error("Color Theme file missing [colors]", "");
        return;
    }

    toml_datum_t keyword_color = toml_int_in(color_table, "keyword_color");
    if (!keyword_color.ok) {
        error("cannot read colors.keyword_color", "");
        return;
    }

    toml_datum_t symbol_color = toml_int_in(color_table, "symbol_color");
    if (!symbol_color.ok) {
        error("cannot read colors.symbol_color", "");
        return;
    }

    toml_datum_t string_literal_double_color = toml_int_in(color_table, "string_literal_double_color");
    if (!string_literal_double_color.ok) {
        error("cannot read colors.string_literal_double_color", "");
        return;
    }

    toml_datum_t string_literal_single_color = toml_int_in(color_table, "string_literal_single_color");
    if (!string_literal_single_color.ok) {
        error("cannot read colors.string_literal_single_color", "");
        return;
    }
    
    toml_datum_t number_color = toml_int_in(color_table, "number_color");
    if (!number_color.ok) {
        error("cannot read colors.number_color", "");
        return;
    }

    toml_datum_t built_in_type_color = toml_int_in(color_table, "built_in_type_color");
    if (!built_in_type_color.ok) {
        error("cannot read colors.built_in_type_color", "");
        return;
    }

    toml_datum_t type_color = toml_int_in(color_table, "type_color");
    if (!type_color.ok) {
        error("cannot read colors.type_color", "");
        return;
    }

    toml_datum_t function_name_color = toml_int_in(color_table, "function_name_color");
    if (!function_name_color.ok) {
        error("cannot read colors.function_name_color", "");
        return;
    }

    toml_datum_t foreground_color = toml_int_in(color_table, "foreground_color");
    if (!foreground_color.ok) {
        error("cannot read colors.foreground_color", "");
        return;
    }

    toml_datum_t background_color = toml_int_in(color_table, "background_color");
    if (!background_color.ok) {
        error("cannot read colors.background_color", "");
        return;
    }

    toml_datum_t comment_single_color = toml_int_in(color_table, "comment_single_color");
    if (!comment_single_color.ok) {
        error("cannot read colors.comment_single_color", "");
        return;
    }

    toml_datum_t comment_multi_color = toml_int_in(color_table, "comment_multi_color");
    if (!comment_multi_color.ok) {
        error("cannot read colors.comment_multi_color", "");
        return;
    }

    theme-> keyword_color = keyword_color.u.i;
    theme-> symbol_color = symbol_color.u.i;
    theme-> string_literal_double_color = string_literal_double_color.u.i;
    theme-> string_literal_single_color = string_literal_single_color.u.i;
    theme-> number_color = number_color.u.i;
    theme-> built_in_type_color = built_in_type_color.u.i;     
    theme-> type_color = type_color.u.i;
    theme-> function_name_color = function_name_color.u.i;
    theme-> foreground_color = foreground_color.u.i;
    theme-> background_color = background_color.u.i;
    theme-> comment_single_color = comment_single_color.u.i;
    theme-> comment_multi_color = comment_multi_color.u.i;

    printf("Loaded color theme: %s\n", path);
    free(color_conf);
}

Config configInit() {
    Config config;
    config.theme_path = NULL;
    return config;
}
void configDestroy(Config *config) {
    if (config->theme_path)
        free(config->theme_path);
}

void loadConfigFromFile(Config *config, const char* path) {
    FILE *fp;
    char errbuf[200];

    fp = fopen(path, "r");
    
    toml_table_t *config_conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!config_conf) {
        error("cannot open editor config file - ", strerror(errno));
        return;
    }

    toml_table_t* editor_table = toml_table_in(config_conf, "editor");
    if (!editor_table) {
        error("Config file missing [editor]", "");
        return;
    }

    // Get data
    toml_datum_t color_theme = toml_string_in(editor_table, "color_theme");
    if (!color_theme.ok) {
        error("cannot read editor.color_theme", "");
        return;
    }
    config->theme_path = color_theme.u.s;
    printf("Theme path: %s\n", config->theme_path);

    free(config_conf);
}
