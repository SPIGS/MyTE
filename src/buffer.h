#pragma once
#include "putils/unicode.h"

#define INITIAL_BUFFER_SIZE 64

typedef struct {
    UnicodeChar *data;
    size_t gap_start;
    size_t gap_end;
    size_t end;
    size_t cstring_size; // The size in bytes needed for a string to fit the content of the buffer
}GapBuffer;

GapBuffer *gapBufferNew(size_t inital_size);
void gapBufferDestroy(GapBuffer *buf);
size_t getBufLength(GapBuffer *buf);
UnicodeChar getBufChar(GapBuffer *buf, size_t cursor);
void shiftGap(GapBuffer *buf, size_t cursor);
void resizeGap(GapBuffer *buf, size_t required_space);
void insertUnicodeCharIntoBuf (GapBuffer *buf, size_t cursor, UnicodeChar uc, size_t grapheme_size);
size_t insertIntoBuf(GapBuffer *buf, size_t cursor, char *bytes);
UnicodeChar removeGraphemeBeforeGap(GapBuffer *buf, size_t cursor);
UnicodeChar removeGraphemeAfterGap(GapBuffer *buf, size_t cursor);
UnicodeChar *getBufferString(GapBuffer *buf);
void outputBufferString(GapBuffer *buf, size_t cursor);

size_t getNextGraphemeCursor(GapBuffer *buf, size_t cursor);
size_t getPrevGraphemeCursor(GapBuffer *buf, size_t cursor);
size_t getBeginningOfLineCursor(GapBuffer *buf, size_t cursor);
size_t getEndOfLineCursor(GapBuffer *buf, size_t cursor);

size_t getBeginningOfNextLineCursor(GapBuffer *buf, size_t cursor);
size_t getEndOfPrevLineCursor(GapBuffer *buf, size_t cursor);
size_t getBeginningOfPrevLineCursor(GapBuffer *buf, size_t cursor);
size_t getBufColumn(GapBuffer *buf, size_t cursor);
size_t getBufLineLength(GapBuffer *buf, size_t cursor);

