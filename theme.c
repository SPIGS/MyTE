#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "theme.h"

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
        LOG_ERROR("cannot open color theme file - ", strerror(errno));
        return;
    }

    toml_table_t *color_table = toml_table_in(color_conf, "colors");
    if (!color_conf) {
        LOG_ERROR("Color Theme file missing [colors]", "");
        return;
    }

    LOAD_TOML_INT(color_table, "keyword_color", keyword_color);
    LOAD_TOML_INT(color_table, "symbol_color", symbol_color);
    LOAD_TOML_INT(color_table, "string_literal_double_color", string_literal_double_color);
    LOAD_TOML_INT(color_table, "string_literal_single_color", string_literal_single_color);
    LOAD_TOML_INT(color_table, "number_color", number_color);
    LOAD_TOML_INT(color_table, "built_in_type_color", built_in_type_color);
    LOAD_TOML_INT(color_table, "type_color", type_color);
    LOAD_TOML_INT(color_table, "function_name_color", function_name_color);
    LOAD_TOML_INT(color_table, "foreground_color", foreground_color);
    LOAD_TOML_INT(color_table, "background_color", background_color);
    LOAD_TOML_INT(color_table, "comment_single_color", comment_single_color);
    LOAD_TOML_INT(color_table, "comment_multi_color", comment_multi_color);

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

    toml_free(color_conf);    
}

Config configInit() {
    Config config;
    config.font_path = NULL;
    config.theme_path = NULL;
    config.tab_stop = 3;
    config.cursor_speed = 3.5;
    return config;
}
void configDestroy(Config *config) {
    if (config->font_path)
        free(config->font_path);

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
        LOG_ERROR("cannot open editor config file - ", strerror(errno));
        return;
    }

    toml_table_t* editor_table = toml_table_in(config_conf, "editor");
    if (!editor_table) {
        LOG_ERROR("Config file missing [editor]", "");
        return;
    }

    // Get data
    LOAD_TOML_STR(editor_table, "font", font_path);
    LOAD_TOML_STR(editor_table, "color_theme", color_theme);
    LOAD_TOML_INT(editor_table, "tab_stop", tab_stop);
    LOAD_TOML_DOUBLE(editor_table, "cursor_speed", cursor_speed);
    config->font_path = font_path.u.s;
    config->theme_path = color_theme.u.s;
    config->tab_stop = tab_stop.u.i;
    config->cursor_speed= cursor_speed.u.d;

    toml_free(config_conf);
}
