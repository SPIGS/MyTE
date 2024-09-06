#pragma once
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define INITIAL_BUFFER_SIZE 4

typedef struct {
    char *data;
    size_t gap_start;
    size_t gap_end;
    size_t end;
} GapBuffer;

GapBuffer *gapBufferInit(size_t inital_size);
void gapBufferDestroy(GapBuffer *buf);
size_t getBufGapSize(GapBuffer *buf);
size_t getBufLength(GapBuffer *buf);
size_t getCursorIdx(GapBuffer *buf, size_t cursor_pos);
char getBufChar(GapBuffer *buf, size_t cursor);
void shiftGap (GapBuffer *buf, size_t cursor);
void resizeGap(GapBuffer *buf, size_t required_space);
void insertCharIntoBuf (GapBuffer *buf, size_t cursor, char c);
void removeCharBeforeGap (GapBuffer *buf, size_t cursor);
char removeCharAfterGap (GapBuffer *buf, size_t cursor);
char *getBufString (GapBuffer *buf);
void outputBufString (GapBuffer *buf, size_t);

size_t getNextCharCursor(GapBuffer *buf, size_t cursor);
size_t getPrevCharCursor(GapBuffer *buf, size_t cursor);
size_t getBeginningOfLineCursor(GapBuffer *buf, size_t cursor);
size_t getEndOfLineCursor(GapBuffer *buf, size_t cursor);
size_t getBeginningOfNextLineCursor(GapBuffer *buf, size_t cursor);
size_t getEndOfPrevLineCursor(GapBuffer *buf, size_t cursor);
size_t getBeginningOfPrevLineCursor(GapBuffer *buf, size_t cursor);
size_t getBufColumn(GapBuffer *buf, size_t cursor);
size_t getBufLineLength(GapBuffer* buf, size_t cursor);