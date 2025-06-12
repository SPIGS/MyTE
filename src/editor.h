#pragma once

#include "buffer.h"
#include "context.h"
#include "cursor.h"
#include "lexer.h"
#include "putils/defines.h"
#include "putils/pmath.h"

#define CURSOR_SPEED 3.5
#define SCROLL_SPEED 3.0

// Signifies what is controlling scroll
typedef enum {
    SCROLL_MODE_CURSOR,
    SCROLL_MODE_MOUSE
} ScrollMode;

typedef struct {
    Vector2 size;
    Vector2 txt_pos;
    i32 padding;
} Gutter;

typedef struct {
    GapBuffer *buf;
    Cursor cursor;
    f32 cursor_speed;
    f32 scroll_speed;
    ScrollMode scroll_mode;

    // Lexer stuff
    Lexer lexer;
    bool dirty;

    // Editor statistics
    i32 goal_col;
    size_t line_count;
    bool unsaved;
    string path;

    // Render info
    Gutter gutter;
    Rect frame;
    Vector2 text_pos;
    Vector2 scroll_pos;
    Vector2 target_scroll_pos;
    f32 line_height;
} Editor;

Editor *editorNew(Rect frame, f32 line_height);
void editorDestroy(Editor *ed);
void editorUpdate(Editor *ed, AppContext *ctx, f64 delta_time);
size_t getBegginingOfCursorLine(Editor *ed);

/* File handling */
void editorSetPath(Editor *ed, const char *path);
void editorLoadFile(Editor *ed, const char *path);
void editorSaveFile(Editor *ed);

/* Control cursor movements */

void editorMoveLeft(Editor *ed);
void editorMoveRight(Editor *ed);
void editorMoveUp(Editor *ed);
void editorMoveDown(Editor *ed);
void editorMoveEndOfNextWord (Editor *ed);
void editorMoveBegOfPrevWord(Editor *ed);

/* Buffer manipulation */

void editorInsert(Editor *ed, char *bytes);
void editorDeleteLeft(Editor *ed);
void editorDeleteRight(Editor *ed);
void editorDeleteWordLeft(Editor *ed);
void editorDeleteWordRight(Editor *ed);

/* Mouse Controls */
void editorScrollWithMouseWheel(Editor *ed, f32 yoffset);
void moveCursorToMousePos(Editor *ed, AppContext *ctx, f64 mouse_x, f64 mouse_y);
