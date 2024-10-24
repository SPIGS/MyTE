#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "gapbuffer.h"
#include "util.h"
#include "lexer.h"
#include "browser.h"
#include "cursor.h"
#include "dialog.h"
#include "context.h"

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
    vec2 screen_pos;
    f32 gutter_width;
    i32 digits;
} Gutter;

Gutter gutterInit(vec2 screen_pos, f32 glyph_adv);

typedef struct {
    FileBrowser browser;
    Gutter gutter;
    SaveDialog sd;

    GapBuffer *buf;
    Cursor cursor;
    i32 goal_column;
    
    EditorMode mode;

    // Lexing stuff
    Lexer lexer;
    bool dirty;

    // Used to draw the editor
    rect frame;
    vec2 text_pos;
    vec2 scroll_pos;
    vec2 target_scroll_pos;
    ScrollMode scroll_mode;

    // Stats to keep track of
    size_t line_count;
    const char *file_path;

    // Configuration stuff
    i32 tab_stop;
    f64 cursor_speed;
    i32 scroll_speed;
    i32 scroll_stop_top;
    i32 scroll_stop_bottom;
      
} Editor;

void editorInit(Editor *ed, rect frame, AppContext *ctx, const char *cur_dir);
void editorDestroy(Editor *ed);
void editorLoadConfig(Editor *ed, Config *config);
void editorChangeMode(Editor *ed, AppContext *ctx, EditorMode new_mode);

void editorLoadFile(Editor *ed, AppContext *ctx, const char *file_path);
void editorWriteFile(Editor *ed);
void editorUpdate(Editor *ed, AppContext *ctx, f64 delta_time);

// Cursor Movements
void editorMoveLeft(Editor *ed);
void editorMoveRight(Editor *ed);
void editorMoveUp(Editor *ed);
void editorMoveDown(Editor *ed);
void editorMoveEndOfNextWord(Editor *ed);
void editorMoveBegOfPrevWord(Editor *ed);

// Buffer manipulation
void editorClearBuffer(Editor *ed);
void editorInsertCharacter(Editor *ed, char character, bool move_cursor_forward);
void editorDeleteCharLeft(Editor *ed);
void editorDeleteCharRight(Editor *ed);
void editorDeleteWordLeft(Editor *ed);
void editorDeleteWordRight(Editor *ed);

// Mouse controls
void moveCursorToMousePos(Editor *ed, AppContext *ctx, vec2 mouse_pos);
void scrollWithMouseWheel(Editor *ed, AppContext *ctx, f32 yoffset);

// Selection manipulation
void editorMakeSelection(Editor *ed);
void editorUnselectSelection(Editor *ed);
void editorDeleteSelection(Editor *ed);