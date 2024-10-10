#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "gapbuffer.h"
#include "util.h"
#include "lexer.h"
#include "browser.h"

#define CURSOR_SPEED 3.5
#define INIT_EDITOR_FRAME rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200)

typedef enum {
    EDITOR_MODE_NORMAL,
    EDITOR_MODE_OPEN,
    EDITOR_MODE_SAVE
} EditorMode;

typedef enum {
    SCROLL_MODE_CURSOR,
    SCROLL_MODE_MOUSE
} ScrollMode;

typedef struct {
    size_t buffer_pos;
    size_t prev_buffer_pos;
    vec2 screen_pos;
    vec2 prev_screen_pos;
    vec2 target_screen_pos;
    f32 anim_time;

    size_t disp_column;
    size_t disp_row;
} Cursor;

Cursor cursorInit();

typedef struct {
    // Buffer / cursor movement
    GapBuffer *buf;
    Cursor cursor;
    i32 goal_column;

    // Used to draw the editor
    rect frame;
    vec2 text_pos;
    vec2 scroll_pos;
    vec2 target_scroll_pos;
    ScrollMode scroll_mode;

    // Stats to keep track of
    size_t line_count;
    f32 line_height;
    const char *file_path;
    f32 glyph_adv;
    f32 descender;

    // Lexing stuff
    Lexer lexer;
    bool dirty;

    // Configuration stuff
    i32 tab_stop;
    f64 cursor_speed;
    i32 scroll_speed;
    i32 scroll_stop_top;
    i32 scroll_stop_bottom;

    // Editor modes
    EditorMode mode;

    // File Browser stuff
    FileBrowser browser;
} Editor;

void editorInit(Editor *ed, rect frame, f32 line_height, f32 glyph_adv, f32 descender, const char *cur_dir);
void editorDestroy(Editor *ed);
void editorLoadConfig(Editor *ed, Config *config);
void editorChangeMode(Editor *ed, EditorMode new_mode);

// Cursor Movements
void moveCursorLeft(Editor *ed);
void moveCursorRight(Editor *ed);
void moveCursorUp(Editor *ed);
void moveCursorDown(Editor *ed);
void insertCharacter(Editor *ed, char character, bool move_cursor_forward);
void deleteCharacterLeft(Editor *ed);
void deleteCharacterRight(Editor *ed);
void editorUpdate(Editor *ed, f32 screen_width, f32 screen_height, f64 delta_time);
void setCursorTargetScreenPos(Editor *ed, vec2 new_target);
void lerpCursorScreenPos(Editor *ed);

char *getContents(Editor *ed);
void loadFromFile(Editor *ed, const char *file_path);
void writeToFile(Editor *ed);
void clearBuffer(Editor *ed);
void moveEndOfNextWord(Editor *ed);
void moveBegOfPrevWord(Editor *ed);
void deleteWordLeft(Editor *ed);
void deleteWordRight(Editor *ed);

void moveCursorToMousePos(Editor *ed, vec2 mouse_pos);
void scrollWithMouseWheel(Editor *ed, f32 yoffset);