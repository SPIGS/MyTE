#pragma once

#include "buffer.h"
#include "cursor.h"
#define CURSOR_SPEED 3.5

// Signifies what is controlling scroll
typedef enum {
    SCROLL_MODE_CURSOR,
    SCROLL_MODE_MOUSE
} ScrollMode;

typedef struct {
    GapBuffer *buf;
    Cursor cursor;
} Editor;

Editor *editorNew(void);
void editorDestroy(Editor *ed);


/* Control cursor movements */

void editorMoveLeft(Editor *ed);
void editorMoveRight(Editor *ed);


/* Buffer manipulation */

void editorInsert(Editor *ed, char *bytes);
void editorDeleteLeft(Editor *ed);
void editorDeleteRight(Editor *ed);

