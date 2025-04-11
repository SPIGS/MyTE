#include <stdio.h>
#include <stdlib.h>
#include "editor.h"
#include "buffer.h"
#include "cursor.h"
#include <grapheme.h>
#include "putils/unicode.h"

Editor *editorNew(Rect frame) {
    Editor *ed = (Editor *)malloc(sizeof(Editor));
    ed->buf = gapBufferNew(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorNew();

    ed->goal_col = -1;
    ed->line_count = 1;

    ed->frame = frame;
    ed->scroll_pos = vec2(0,0);
    ed->target_scroll_pos = vec2(0,0);
    return ed;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
    free(ed);
}

size_t getBegginingOfCursorLine(Editor *ed) {
    return getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx);
}

void editorMoveLeft(Editor *ed) {
    ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);

    // If we moved to the previous line
    if (getBufChar(ed->buf, ed->cursor.buffer_idx) == 10) {
        ed->cursor.disp_row--;
    }

    ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    ed->goal_col = ed->cursor.disp_col;

    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}

void editorMoveRight(Editor *ed) {
    if (ed->cursor.buffer_idx == getBufLength(ed->buf)) {
        return;
    }

    ed->cursor.buffer_idx = getNextGraphemeCursor(ed->buf, ed->cursor.buffer_idx);

    // If we moved to the next line
    if (ed->cursor.buffer_idx == getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx)) {
        ed->cursor.disp_row++;
    }

    ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    ed->goal_col = ed->cursor.disp_col;

    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}

void editorMoveUp(Editor *ed) {
    if (ed->goal_col == -1) {
        ed->goal_col = ed->cursor.disp_col;
    }

    size_t beg_prev_line = getBeginningOfPrevLineCursor(ed->buf, ed->cursor.buffer_idx);
    size_t beg_line = getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx);

    if (beg_line == 0) {
        ed->cursor.buffer_idx = beg_line;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
        ed->goal_col = ed->cursor.disp_col;
    } else {
        size_t len_prev_line = getBufLineLength(ed->buf, beg_prev_line);
        ed->cursor.buffer_idx = beg_prev_line + MIN((size_t)ed->goal_col - 1, len_prev_line);
        ed->cursor.disp_row--;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    }
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}

void editorMoveDown(Editor *ed) {
    if (ed->goal_col == -1) {
        ed->goal_col = ed->cursor.disp_col;
    }

    size_t beg_next_line = getBeginningOfNextLineCursor(ed->buf, ed->cursor.buffer_idx);
    size_t end_line = getEndOfLineCursor(ed->buf, ed->cursor.buffer_idx);

    if (end_line == getBufLength(ed->buf)) {
        ed->cursor.buffer_idx = beg_next_line;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
        ed->goal_col = ed->cursor.disp_col;
    } else {
        size_t len_next_line = getBufLineLength(ed->buf, beg_next_line);
        ed->cursor.buffer_idx = beg_next_line + MIN((size_t)ed->goal_col - 1, len_next_line);
        ed->cursor.disp_row++;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    }
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}

void editorInsert(Editor *ed, char *bytes) {
    
    size_t grapheme_size, offset = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);

        insertUnicodeCharIntoBuf (ed->buf, ed->cursor.buffer_idx, grapheme, grapheme_size);
        if (grapheme == '\n') { // new line
            ed->line_count++;
            ed->cursor.disp_row++;
            
        }
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
        ed->goal_col = ed->cursor.disp_col;
        ed->cursor.buffer_idx ++;
    }

    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu\n", ed->line_count);
}

void editorDeleteLeft(Editor *ed) {
    // TODO: simplify this (remove redundant code)
    if (ed->cursor.buffer_idx != 0) {
        if (getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx)) != '\n') {
            removeGraphemeBeforeGap(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
            ed->goal_col = ed->cursor.disp_col;
        } else {
            removeGraphemeBeforeGap(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
            ed->goal_col = ed->cursor.disp_col;
            ed->cursor.disp_row --;
            ed->line_count --;
        }
    }
    
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}

void editorDeleteRight(Editor *ed) {
    UnicodeChar removed_char = removeGraphemeAfterGap(ed->buf, ed->cursor.buffer_idx);

    if (removeGraphemeAfterGap(ed->buf, ed->cursor.buffer_idx) == '\n') {
        ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
    }

    outputBufferString(ed->buf, ed->cursor.buffer_idx);
    printf("Num limes: %lu", ed->line_count);
}
