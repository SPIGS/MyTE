#include "gapbuffer.h"
#include<stdio.h>
#include <assert.h>


GapBuffer *gapBufferInit(size_t inital_size) {
    GapBuffer *buf = (GapBuffer*)malloc(sizeof(GapBuffer));
    buf->data = (char*)malloc(inital_size * sizeof(char));
    buf->gap_start = 0;
    buf->gap_end = inital_size;
    buf->end = inital_size;
    return buf;
}

static void assertBufferInvariants(GapBuffer* buf) {
	assert(buf->data);
	assert(buf->gap_start <= buf->gap_end);
	assert(buf->gap_end <= buf->end);
}

static void assertCursorInvariants(GapBuffer* buf, size_t cursor) {
	assert(cursor <= getBufLength(buf));
}

void gapBufferDestroy(GapBuffer *buf) {
    free(buf->data);
    free(buf);
}

size_t getBufGapSize(GapBuffer *buf) {
    return buf->gap_end - buf->gap_start;
}

size_t getBufLength(GapBuffer *buf) {
    return buf->end - getBufGapSize(buf);
}

size_t getCursorIdx(GapBuffer *buf, size_t cursor_pos) {
    return ((cursor_pos <= buf->gap_start) ? cursor_pos : cursor_pos + getBufGapSize(buf));
}

char getBufChar(GapBuffer *buf, size_t cursor) {
    return buf->data[getCursorIdx(buf, cursor)];
}

void shiftGap (GapBuffer *buf, size_t cursor) {
    size_t gap_size = getBufGapSize(buf);
    if (cursor < buf->gap_start) {
        size_t move_size = buf->gap_start - cursor;
        buf->gap_start -= move_size;
        buf->gap_end -= move_size;
        memmove(buf->data + buf->gap_end, buf->data + buf->gap_start, move_size);
    } else if (cursor > buf->gap_start) {
        size_t move_size = cursor - buf->gap_start;
        memmove(buf->data + buf->gap_start, buf->data + buf->gap_end, move_size);
        buf->gap_start += move_size;
        buf->gap_end += move_size;
    }
    assert(getBufGapSize(buf) == gap_size);
	assertBufferInvariants(buf);
}

void resizeGap(GapBuffer *buf, size_t required_space) {
    if (getBufGapSize(buf) < required_space) {
        shiftGap(buf, getBufLength(buf));
        size_t new_end = MAX(2 * buf->end, buf->end + required_space) - getBufGapSize(buf);
        buf->data = (char *)realloc(buf->data, new_end * sizeof(char));
        buf->end = new_end;
        buf->gap_end = buf->end;       
    }
    assert(getBufGapSize(buf) >= required_space);
}

void insertCharIntoBuf (GapBuffer *buf, size_t cursor, char c) {
    assertCursorInvariants(buf, cursor);
    resizeGap(buf, 1);
    shiftGap(buf, cursor);
    buf->data[buf->gap_start] = c;
    buf->gap_start++;
    shiftGap(buf, cursor);
}

void removeCharBeforeGap (GapBuffer *buf, size_t cursor) {
    if (cursor > 0) {
        shiftGap(buf, cursor);
        buf->gap_start--;
    }
}

char removeCharAfterGap(GapBuffer *buf, size_t cursor) {
    if (cursor < getBufLength(buf)) {
        shiftGap(buf, cursor);
        char removed_char = buf->data[buf->gap_end];
        buf->gap_end++;
        return removed_char;
    }
    return -1;
}

char *getBufString (GapBuffer *buf) {
    size_t length = getBufLength(buf);
    char *temp = (char *)malloc((length + 1) * sizeof(char)); // +1 for null-terminator
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(temp, buf->data, buf->gap_start); // Copy before the gap
    memcpy(temp + buf->gap_start, buf->data + buf->gap_end, buf->end - buf->gap_end); // Copy after the gap

    temp[length] = '\0'; // Null-terminate the string
    return temp;
}

void outputBufferString (GapBuffer *buf, size_t cursor) {
    char *temp = getBufString(buf);
    printf("%lu, %lu\n", cursor, getCursorIdx(buf, cursor));
    printf("%s\n", temp);
    free(temp);
}

size_t getNextCharCursor(GapBuffer *buf, size_t cursor) {
    assertCursorInvariants(buf, cursor);
    if (cursor < getBufLength(buf)) {
        return cursor + 1;
    } else {
        return cursor;
    }
}

size_t getPrevCharCursor(GapBuffer *buf, size_t cursor) {
    UNUSED(buf);
    if (cursor > 0) {
        return cursor - 1;
    } else {
        return cursor;
    }
}

size_t getBeginningOfLineCursor(GapBuffer *buf, size_t cursor) {
    cursor = getPrevCharCursor(buf, cursor);
    while (cursor > 0) {
        char character = getBufChar(buf, cursor);
        if (character == '\n') {
            return getNextCharCursor(buf, cursor);
        }
        cursor = getPrevCharCursor(buf, cursor);
    }
    return 0;
}

size_t getEndOfLineCursor(GapBuffer *buf, size_t cursor) {
    while (cursor < getBufLength(buf)) {
        char character = getBufChar(buf, cursor);
        if (character == '\n') {
            return cursor;
        }
        cursor = getNextCharCursor(buf, cursor);
    }
    return getBufLength(buf);
}

size_t getBeginningOfNextLineCursor(GapBuffer *buf, size_t cursor) {
    return getNextCharCursor(buf, getEndOfLineCursor(buf, cursor));
}

size_t getEndOfPrevLineCursor(GapBuffer *buf, size_t cursor) {
    return getPrevCharCursor(buf, getBeginningOfLineCursor(buf, cursor));
}

size_t getBeginningOfPrevLineCursor(GapBuffer *buf, size_t cursor) {
    return getBeginningOfLineCursor(buf, getPrevCharCursor(buf, getBeginningOfLineCursor(buf, cursor)));
}

size_t getBufColumn(GapBuffer *buf, size_t cursor) {
    return cursor - getBeginningOfLineCursor(buf, cursor);
}

size_t getBufLineLength(GapBuffer* buf, size_t cursor) {
    size_t end = getEndOfLineCursor(buf, cursor);
    size_t beg = getBeginningOfLineCursor(buf, cursor);
	return end - beg;
}