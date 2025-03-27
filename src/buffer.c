#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <grapheme.h>
#include <stdio.h>

#include "buffer.h"
#include "putils/pmath.h"


GapBuffer *gapBufferNew(size_t inital_size) {
    GapBuffer *buf = (GapBuffer *)malloc(sizeof(GapBuffer));
    *buf = (GapBuffer) {
        NULL,
        0,
        inital_size,
        inital_size,
        0
    };
    buf->data = (UnicodeChar*)malloc(inital_size * sizeof(UnicodeChar));
    return buf;
}

void gapBufferDestroy(GapBuffer *buf) {
    free(buf->data);
}

static size_t getBufGapSize(GapBuffer *buf) {
    return buf->gap_end - buf->gap_start;
}

// TODO(parker): rename to to something like "getBufCapacity"
size_t getBufLength(GapBuffer *buf) {
    return buf->end - getBufGapSize(buf);
}

static void assertBufferInvariants(GapBuffer *buf) {
    assert(buf->data);
    assert(buf->gap_start <= buf->gap_end);
    assert(buf->gap_end <= buf->end);
}

static void assertCursorInvariants(GapBuffer *buf, size_t cursor) {
    assert(cursor <= getBufLength(buf));
}

static size_t getCursorIdx(GapBuffer *buf, size_t cursor) {
    return ((cursor < buf->gap_start) ? cursor : cursor + getBufGapSize(buf));
}

UnicodeChar getBufChar(GapBuffer *buf, size_t cursor) {
    return buf->data[getCursorIdx(buf, cursor)];
}

void shiftGap(GapBuffer *buf, size_t cursor) {
    size_t gap_size = getBufGapSize(buf);
    if (cursor < buf->gap_start) {
        size_t move_size = buf->gap_start - cursor;
        buf->gap_start -= move_size;
        buf->gap_end -= move_size;
        memmove(buf->data + buf->gap_end, buf->data + buf->gap_start, move_size * sizeof(UnicodeChar));
    } else if (cursor > buf->gap_start) {
        size_t move_size = cursor - buf->gap_start;
        memmove(buf->data + buf->gap_start, buf->data + buf->gap_end, move_size * sizeof(UnicodeChar));
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
        buf->data = (UnicodeChar*)realloc(buf->data, new_end * sizeof(UnicodeChar));
        buf->end = new_end;
        buf->gap_end = buf->end;
    }
    assert(getBufGapSize(buf) >= required_space);
}

void insertUnicodeCharIntoBuf (GapBuffer *buf, size_t cursor, UnicodeChar uc) {
    assertCursorInvariants(buf, cursor);
    resizeGap(buf, 1);
    shiftGap(buf, cursor);
    buf->data[buf->gap_start] = uc;
    buf->gap_start++;
}

//Assumes bytes is a null-terminated string
size_t insertIntoBuf(GapBuffer *buf, size_t cursor, char *bytes) {
    size_t grapheme_size, offset, num_graphemes = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);
        insertUnicodeCharIntoBuf(buf, cursor + num_graphemes, grapheme);
        buf->cstring_size += grapheme_size;
        num_graphemes++;
    }

    return cursor + num_graphemes;
}

size_t getNextGraphemeCursor(GapBuffer *buf, size_t cursor) {
    assertCursorInvariants(buf, cursor);
    if (cursor < getBufLength(buf)) {
        return cursor + 1;
    } else {
        return cursor;
    }
}

size_t getPrevGraphemeCursor(GapBuffer *buf, size_t cursor) {
    UNUSED(buf);
    if (cursor > 0) {
        return cursor - 1;
    } else {
        return cursor;
    }
}

void removeGraphemeBeforeGap(GapBuffer *buf, size_t cursor) {
    if (cursor > 0) {
        shiftGap(buf, cursor);
        buf->gap_start--;
        buf->cstring_size -= getUTF8Size(buf->data[buf->gap_start]);
    }
}

UnicodeChar removeGraphemeAfterGap(GapBuffer *buf, size_t cursor) {
    if (cursor < getBufLength(buf)) {
        shiftGap(buf, cursor);
        UnicodeChar removed_char = buf->data[buf->gap_end];
        buf->gap_end++;
        buf->cstring_size -= getUTF8Size(removed_char);
        return removed_char;
    }
    return 0;
}

void outputBufferString(GapBuffer *buf, size_t cursor) {
    printf("\n");
    size_t left_len = buf->gap_start;
    for (size_t i = 0; i < left_len; i++) {
        char *bytes = unpackUTF8(buf->data[i]);
        if (cursor == i)
            printf("|");

        printf("%s", bytes);

        if (bytes)
            free(bytes);
    }

    size_t right_len = (buf->end - buf->gap_end);
    for (size_t i = 0; i < right_len; i++) {
        char *bytes = unpackUTF8(buf->data[buf->gap_end + i]);
        if (cursor ==  left_len+ i)
            printf("|");

        printf("%s", bytes);

        if (bytes)
            free(bytes);
    }

    if (cursor >= right_len + left_len) {
            printf("|");
    }

    printf("\n");

    printf("gap start: %zu, gap end: %zu, cursor: %zu\n", buf->gap_start, buf->gap_end, cursor);
    printf("Memory foot print: %zu bytes\n", buf->end * 4);
    printf("space needed for a byte string: %zu bytes\n", buf->cstring_size);
}

size_t getBeginningOfLineCursor(GapBuffer *buf, size_t cursor) {
    while (cursor > 0) {
        UnicodeChar c =  getBufChar(buf, getPrevGraphemeCursor(buf, cursor));
        if (c == 10) {
            return cursor;
        }
        cursor = getPrevGraphemeCursor(buf, cursor);
    }
    return 0;
}

size_t getEndOfLineCursor(GapBuffer *buf, size_t cursor) {
    while (cursor < getBufLength(buf)) {
        UnicodeChar c = getBufChar(buf, cursor);
        if (c == 10) {
            return cursor;
        }
        cursor = getNextGraphemeCursor(buf, cursor);
    }
    return getBufLength(buf);
}
size_t getBeginningOfNextLineCursor(GapBuffer *buf, size_t cursor) {
    return getNextGraphemeCursor(buf, getEndOfLineCursor(buf, cursor));
}

size_t getEndOfPrevLineCursor(GapBuffer *buf, size_t cursor) {
    return getPrevGraphemeCursor(buf, getBeginningOfLineCursor(buf, cursor));
}

size_t getBeginningOfPrevLineCursor(GapBuffer *buf, size_t cursor) {
    return getBeginningOfLineCursor(buf, getPrevGraphemeCursor(buf, getBeginningOfLineCursor(buf, cursor)));
}

size_t getBufColumn(GapBuffer *buf, size_t cursor) {
    return cursor - getBeginningOfLineCursor(buf, cursor);
}
size_t getBufLineLength(GapBuffer *buf, size_t cursor) {
    size_t end = getEndOfLineCursor(buf, cursor);
    size_t beg = getBeginningOfLineCursor(buf, cursor);
    return end - beg;
}
