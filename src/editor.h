#pragma once

#include "buffer.h"
#include "cursor.h"
#include "putils/defines.h"
#include "putils/pmath.h"
#include "putils/unicode.h"

#define CURSOR_SPEED 3.5

// Signifies what is controlling scroll
typedef enum {
    SCROLL_MODE_CURSOR,
    SCROLL_MODE_MOUSE
} ScrollMode;

typedef struct {
    GapBuffer *buf;
    Cursor cursor;

    // Editor statistics
    i32 goal_col;
    size_t line_count;

    // Render info
    Rect frame;
    Vector2 text_pos;
    Vector2 scroll_pos;
    Vector2 target_scroll_pos;
} Editor;

Editor *editorNew(Rect frame);
void editorDestroy(Editor *ed);
size_t getBegginingOfCursorLine(Editor *ed);

/* Control cursor movements */

void editorMoveLeft(Editor *ed);
void editorMoveRight(Editor *ed);
void editorMoveUp(Editor *ed);
void editorMoveDown(Editor *ed);


/* Buffer manipulation */

void editorInsert(Editor *ed, char *bytes);
void editorDeleteLeft(Editor *ed);
void editorDeleteRight(Editor *ed);

