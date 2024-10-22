#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "config.h"

/* DEFAULT GENREAL SETTINGS */
#define DEFAULT_FONT_PATH "./fonts/iosevka-firamono.ttf"
#define DEFAULT_THEME_PATH "./config/themes/spaceduck.toml"
#define DEFAULT_FONT_SIZE 24
#define DEFAULT_SHOW_FPS false 

/* DEFAULT EDITOR SETTINGS */
#define DEFAULT_TAB_STOP 3
#define DEFAULT_CURSOR_SPEED 6.5
#define DEFAULT_SCROLL_SPEED 1
#define DEFAULT_SCROLL_STOP_TOP 6
#define DEFAULT_SCROLL_STOP_BOTTOM 2

/* DEFAULT GENREAL COLORS */
#define DEFAULT_FOREGROUND_COLOR color_from_hex(0xECf0C1FF)
#define DEFAULT_BACKGROUND_COLOR color_from_hex(0x0F111BFF)
#define DEFAULT_CURRENT_LINE_COLOR color_from_hex(0x16172DFF)
#define DEFAULT_HIGHLIGHT_COLOR color_from_hex(0x30365FFF)
#define DEFAULT_GUTTER_FOREGROUND_COLOR color_from_hex(0x30365FFF)

/* DEFAULT SYNTAX COLORS*/
#define DEFAULT_KEYWORD_COLOR color_from_hex(0x7A5CCCFF)
#define DEFAULT_SECONDARY_KEYWORD_COLOR color_from_hex(0xB3A1E6FF)
#define DEFAULT_BUILT_IN_TYPE_COLOR color_from_hex(0x5CCC96FF)
#define DEFAULT_TYPE_COLOR color_from_hex(0xF2CE00FF) 
#define DEFAULT_FUNCTION_NAME_COLOR color_from_hex(0x5CCC96FF)
#define DEFAULT_SYMBOL_COLOR color_from_hex(0xCE6F8FFF)
#define DEFAULT_STRING_LITERAL_DOUBLE_COLOR color_from_hex(0x00A3CCFF)
#define DEFAULT_STRING_LITERAL_SINGLE_COLOR color_from_hex(0xF2CE00FF)
#define DEFAULT_NUMBER_COLOR color_from_hex(0xF2CE00FF)
#define DEFAULT_COMMENT_SINGLE_COLOR color_from_hex(0x686f9aFF)
#define DEFAULT_COMMENT_MULTI_COLOR color_from_hex(0x686f9aFF)

ColorTheme colorThemeInit() {
    ColorTheme theme;
    
    // General
    theme.foreground = DEFAULT_FOREGROUND_COLOR;
    theme.background = DEFAULT_BACKGROUND_COLOR;
    theme.current_line = DEFAULT_CURRENT_LINE_COLOR;
    theme.user_selection = DEFAULT_HIGHLIGHT_COLOR;
    theme.gutter_foreground = DEFAULT_GUTTER_FOREGROUND_COLOR;

    // Syntax
    theme.keyword = DEFAULT_KEYWORD_COLOR;
    theme.secondary_keyword = DEFAULT_SECONDARY_KEYWORD_COLOR;
    theme.built_in_type = DEFAULT_BUILT_IN_TYPE_COLOR;
    theme.type = DEFAULT_TYPE_COLOR;
    theme.function_name = DEFAULT_FUNCTION_NAME_COLOR;
    theme.symbol = DEFAULT_SYMBOL_COLOR;
    theme.double_quote_string = DEFAULT_STRING_LITERAL_DOUBLE_COLOR;
    theme.single_quote_string = DEFAULT_STRING_LITERAL_SINGLE_COLOR;
    theme.number = DEFAULT_NUMBER_COLOR;
    theme.single_line_comment = DEFAULT_COMMENT_SINGLE_COLOR;
    theme.multiline_comment = DEFAULT_COMMENT_MULTI_COLOR;
    return theme;
}

void colorThemeLoad(ColorTheme *theme, const char *path) {
    FILE *fp;
    char errbuf[200];

    fp = fopen(path, "r");
    
    toml_table_t *theme_file = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!theme_file) {
        LOG_ERROR("Cannot open theme file \'%s\'", path);
        return;
    }

    toml_table_t *general_table = toml_table_in(theme_file, "general");
    if (!general_table) {
        LOG_ERROR("Theme file \'%s\' missing [general].", path);
        return;
    }

    toml_table_t *syntax_table = toml_table_in(theme_file, "syntax");
    if (!syntax_table) {
        LOG_ERROR("Theme file \'%s\' missing [syntax].", path);
        return;
    }

    LOAD_TOML_INT(general_table, foreground);
    LOAD_TOML_INT(general_table, background);
    LOAD_TOML_INT(general_table, current_line);
    LOAD_TOML_INT(general_table, user_selection);
    LOAD_TOML_INT(general_table, gutter_foreground);

    LOAD_TOML_INT(syntax_table, keyword);
    LOAD_TOML_INT(syntax_table, secondary_keyword);
    LOAD_TOML_INT(syntax_table, built_in_type);
    LOAD_TOML_INT(syntax_table, type);
    LOAD_TOML_INT(syntax_table, function_name);
    LOAD_TOML_INT(syntax_table, symbol);
    LOAD_TOML_INT(syntax_table, double_quote_string);
    LOAD_TOML_INT(syntax_table, single_quote_string);
    LOAD_TOML_INT(syntax_table, number);
    LOAD_TOML_INT(syntax_table, single_line_comment);
    LOAD_TOML_INT(syntax_table, multiline_comment);

    theme->foreground = foreground.ok ? color_from_hex(foreground.u.i) : DEFAULT_FOREGROUND_COLOR;
    theme->background = background.ok ? color_from_hex(background.u.i) : DEFAULT_BACKGROUND_COLOR;
    theme->current_line = current_line.ok ? color_from_hex(current_line.u.i) : DEFAULT_CURRENT_LINE_COLOR;
    theme->user_selection = user_selection.ok ? color_from_hex(user_selection.u.i) : DEFAULT_HIGHLIGHT_COLOR;
    theme->gutter_foreground = gutter_foreground.ok ? color_from_hex(gutter_foreground.u.i) : DEFAULT_GUTTER_FOREGROUND_COLOR;

    theme->keyword = keyword.ok ? color_from_hex(keyword.u.i) : DEFAULT_KEYWORD_COLOR;
    theme->secondary_keyword = secondary_keyword.ok ? color_from_hex(secondary_keyword.u.i) : DEFAULT_SECONDARY_KEYWORD_COLOR;
    theme->built_in_type = built_in_type.ok ? color_from_hex(built_in_type.u.i) : DEFAULT_BUILT_IN_TYPE_COLOR;
    theme->type = type.ok ? color_from_hex(type.u.i) : DEFAULT_TYPE_COLOR;
    theme->function_name = function_name.ok ? color_from_hex(function_name.u.i) : DEFAULT_FUNCTION_NAME_COLOR;
    theme->symbol = symbol.ok ? color_from_hex(symbol.u.i) : DEFAULT_SYMBOL_COLOR;
    theme->double_quote_string = double_quote_string.ok ? color_from_hex(double_quote_string.u.i) : DEFAULT_STRING_LITERAL_DOUBLE_COLOR;
    theme->single_quote_string = single_quote_string.ok ? color_from_hex(single_quote_string.u.i) : DEFAULT_STRING_LITERAL_SINGLE_COLOR;
    theme->number = number.ok ? color_from_hex(number.u.i) : DEFAULT_NUMBER_COLOR;
    theme->single_line_comment = single_line_comment.ok ? color_from_hex(single_line_comment.u.i) : DEFAULT_COMMENT_SINGLE_COLOR;
    theme->multiline_comment = multiline_comment.ok ? color_from_hex(multiline_comment.u.i) : DEFAULT_COMMENT_MULTI_COLOR;

    toml_free(theme_file);    
}

Config configInit() {
    Config config;
    config.font_path = NULL;
    config.theme_path = NULL;
    config.tab_stop = 3;
    config.cursor_speed = 3.5;
    config.numCommandConfigs = 0;
    return config;
}
void configDestroy(Config *config) {
    if (config->font_path)
        free(config->font_path);

    if (config->theme_path)
        free(config->theme_path);

    for (size_t i = 0; i < config->numCommandConfigs; i++) {
        if (config->commandConfigs[i].command_name) {
            free(config->commandConfigs[i].command_name);
        }
    }
}

void loadConfigFromFile(Config *config, const char* path) {
    FILE *fp;
    char errbuf[200];

    fp = fopen(path, "r");
    
    toml_table_t *config_file = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!config_file) {
        LOG_ERROR("cannot open editor config file - %c", strerror(errno));
        return;
    }

    toml_table_t* general_table = toml_table_in(config_file, "general");
    if (!general_table) {
        LOG_ERROR("Config file missing [general]", "");
        return;
    }

    toml_table_t* editor_table = toml_table_in(config_file, "editor");
    if (!editor_table) {
        LOG_ERROR("Config file missing [editor]", "");
        return;
    }

    // Get data
    LOAD_TOML_STR(general_table, font);
    LOAD_TOML_STR(general_table, theme);
    LOAD_TOML_INT(general_table, font_size);
    LOAD_TOML_BOOL(general_table, show_fps);

    LOAD_TOML_INT(editor_table, tab_stop);
    LOAD_TOML_DOUBLE(editor_table, cursor_speed);
    LOAD_TOML_INT(editor_table, scroll_speed);
    LOAD_TOML_INT(editor_table, scroll_stop_top);
    LOAD_TOML_INT(editor_table, scroll_stop_bottom);

    config->font_path = font.ok ? font.u.s : DEFAULT_FONT_PATH;
    config->theme_path = theme.ok ? theme.u.s : DEFAULT_THEME_PATH;
    config->font_size = font_size.ok ? font_size.u.i : DEFAULT_FONT_SIZE;
    config->show_fps = show_fps.ok ? show_fps.u.b : DEFAULT_SHOW_FPS;

    config->tab_stop = tab_stop.ok ? tab_stop.u.i : DEFAULT_TAB_STOP;
    config->cursor_speed = cursor_speed.ok ? cursor_speed.u.d : DEFAULT_CURSOR_SPEED;
    config->scroll_speed = scroll_speed.ok ? scroll_speed.u.i : DEFAULT_SCROLL_SPEED;
    config->scroll_speed = scroll_speed.ok ? scroll_speed.u.i : DEFAULT_SCROLL_STOP_TOP;
    config->scroll_stop_bottom = scroll_stop_bottom.ok ? scroll_stop_bottom.u.i : DEFAULT_SCROLL_STOP_BOTTOM;

    // Get Command Configs
    toml_table_t* keybinds_table = toml_table_in(config_file, "keybind");
    if (!keybinds_table) {
        LOG_ERROR("Config file missing [keybind]", "");
        return;
    }

    // for each mode
    for (int i = 0; ; i++) {
        const char *mode = toml_key_in(keybinds_table, i);
        if (!mode) break;
        toml_table_t *mode_keybind_table = toml_table_in(keybinds_table, mode);

        // for each command for the mode
        for (int j = 0; ; j++) {
            const char *command = toml_key_in(mode_keybind_table, j);
            if (!command) break;
            toml_table_t *command_table = toml_table_in(mode_keybind_table, command);

            LOAD_TOML_STR(command_table, key);
            LOAD_TOML_STR_ARRAY(command_table, "mods", mods_array, mods_array_len, mods, mods_count);

            // get mods
            int mod_bitmask = 0;
            for (size_t k = 0; k < mods_count; k++) {
                if (strcmp(mods[k], "alt") == 0) {
                    mod_bitmask = mod_bitmask | GLFW_MOD_ALT;
                } else if (strcmp(mods[k], "control") == 0) {
                    mod_bitmask = mod_bitmask | GLFW_MOD_CONTROL;
                } else if (strcmp(mods[k], "shift") == 0) {
                    mod_bitmask = mod_bitmask | GLFW_MOD_SHIFT;
                } else if (strcmp(mods[k], "super") == 0) {
                    mod_bitmask = mod_bitmask | GLFW_MOD_SUPER;
                }
            }
            int key_code = getKeyFromString(key.u.s);
            config->commandConfigs[config->numCommandConfigs].key = key_code;
            config->commandConfigs[config->numCommandConfigs].mods = mod_bitmask;
            config->commandConfigs[config->numCommandConfigs].command_name = (char *)malloc(sizeof(char) * (strlen(command) + 1));
            strcpy(config->commandConfigs[config->numCommandConfigs].command_name, command);

            if (strcmp(mode, "editor") == 0) {
                config->commandConfigs[config->numCommandConfigs].mode = COMMAND_TYPE_EDITOR;
            } else if (strcmp(mode, "browser") == 0){
                config->commandConfigs[config->numCommandConfigs].mode = COMMAND_TYPE_BROWSER;
            } else {
                config->commandConfigs[config->numCommandConfigs].mode = COMMAND_TYPE_GLOBAL;
            }

            config->numCommandConfigs++;

            free(key.u.s);
            for (size_t k = 0; k < mods_count; k++) {
                if (mods[k]) {
                    free(mods[k]);
                }
            }
            free(mods);
        }
    }

    toml_free(config_file);
}
