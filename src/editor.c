#include <stdio.h>
#include <stdlib.h>
#include "editor.h"
#include "buffer.h"
#include "cursor.h"
#include <grapheme.h>
#include "putils/pmath.h"
#include "putils/unicode.h"
#include "putils/log.h"

Editor *editorNew(Rect frame, f32 line_height) {
    Editor *ed = (Editor *)malloc(sizeof(Editor));
    ed->buf = gapBufferNew(INITIAL_BUFFER_SIZE);
    ed->goal_col = -1;
    ed->line_count = 1;

    ed->gutter = (Gutter) {
        .size = vec2(0.0, 0.0),
        .txt_pos = vec2(0.0, 0.0)
    };

    ed->frame = frame;
    ed->scroll_pos = vec2(0.0,0.0);
    ed->target_scroll_pos = vec2(0.0,0.0);
    ed->line_height = line_height;

    Vector2 pos = vec2(0, 0);
    pos.x = (frame.x + (frame.w / 2.0));
    pos.y = (frame.x + (frame.h / 2.0));

    ed->cursor = cursorNew(pos);
    ed->cursor_speed = CURSOR_SPEED;
    return ed;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
    free(ed);
}

void editorUpdate(Editor *ed, f64 delta_time) {
    cursorUpdate(&ed->cursor, delta_time);

    // Get the offset from the scroll position
    ed->target_scroll_pos = vec2(ed->scroll_pos.x, ed->scroll_pos.y);

    f32 bottom_scroll_bound = ed->frame.y + (6.0 * ed->line_height);
    f32 top_scroll_bound = ed->frame.y + ed->frame.h - (6.0 * ed->line_height);
    f32 bottom_line_y = ed->line_height * ed->line_count;

    // Get the vertical scroll position
    if (ed->cursor.target_screen_pos.y <= bottom_scroll_bound && ed->scroll_pos.y < (bottom_line_y)) {
        ed->target_scroll_pos.y += ed->line_height;
    } else if (ed->cursor.target_screen_pos.y >= top_scroll_bound && ed->scroll_pos.y > 0.0) {
        ed->target_scroll_pos.y -= ed->line_height;
    }

    // Get the horizontal scroll position
    // NOTE: this is for some padding so that the text doesn't sit on top of the divider line
    f32 glyph_width_padding = 12.0;
    if (ed->cursor.target_screen_pos.x > (ed->frame.x + ed->frame.w - 3.0)) {
        ed->target_scroll_pos.x -= (ed->cursor.target_screen_pos.x - (ed->frame.x + ed->frame.w) + 3.0);
    } else if (ed->cursor.target_screen_pos.x < (ed->frame.x + ed->gutter.size.x + glyph_width_padding) && ed->scroll_pos.x < 0) {
        ed->target_scroll_pos.x += (ed->frame.x + ed->gutter.size.x + glyph_width_padding) - ed->cursor.target_screen_pos.x;
    }

    // Get the horizontal scroll position
    ed->scroll_pos = vec2Lerp(ed->scroll_pos, ed->target_scroll_pos, (f32)delta_time  * 35.0f);
}

size_t getBegginingOfCursorLine(Editor *ed) {
    return getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx);
}

void editorMoveLeft(Editor *ed) {
    LOG_DEBUG("Moved left", "");
    ed->cursor.moved_last_frame = true;
    if (ed->cursor.buffer_idx == 0) {
        ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
        return;
    }

    ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
    ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);

    // If we moved to the previous line
    if (getBufChar(ed->buf, ed->cursor.buffer_idx) == 10) {
        ed->cursor.prev_disp_row = ed->cursor.disp_row;
        ed->cursor.disp_row--;
    }

    ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    ed->goal_col = ed->cursor.disp_col;
    ed->cursor.pos_anim_time = 0.0f;

}

void editorMoveRight(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    if (ed->cursor.buffer_idx == getBufLength(ed->buf)) {
        ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
        return;
    }

    ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
    ed->cursor.buffer_idx = getNextGraphemeCursor(ed->buf, ed->cursor.buffer_idx);

    // If we moved to the next line
    if (ed->cursor.buffer_idx == getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx)) {
        ed->cursor.prev_disp_row = ed->cursor.disp_row;
        ed->cursor.disp_row++;
    }

    ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    ed->goal_col = ed->cursor.disp_col;

    ed->cursor.pos_anim_time = 0.0f;
}

void editorMoveUp(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    if (ed->goal_col == -1) {
        ed->goal_col = ed->cursor.disp_col;
    }

    size_t beg_prev_line = getBeginningOfPrevLineCursor(ed->buf, ed->cursor.buffer_idx);
    size_t beg_line = getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx);
    ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;

    if (beg_line == 0) {
        ed->cursor.buffer_idx = beg_line;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
        ed->goal_col = ed->cursor.disp_col;
    } else {
        size_t len_prev_line = getBufLineLength(ed->buf, beg_prev_line);
        ed->cursor.buffer_idx = beg_prev_line + MIN((size_t)ed->goal_col - 1, len_prev_line);
        ed->cursor.prev_disp_row = ed->cursor.disp_row;
        ed->cursor.disp_row--;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorMoveDown(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    if (ed->goal_col == -1) {
        ed->goal_col = ed->cursor.disp_col;
    }

    size_t beg_next_line = getBeginningOfNextLineCursor(ed->buf, ed->cursor.buffer_idx);
    size_t end_line = getEndOfLineCursor(ed->buf, ed->cursor.buffer_idx);
    ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;

    if (end_line == getBufLength(ed->buf)) {
        ed->cursor.buffer_idx = beg_next_line;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
        ed->goal_col = ed->cursor.disp_col;
    } else {
        size_t len_next_line = getBufLineLength(ed->buf, beg_next_line);
        ed->cursor.buffer_idx = beg_next_line + MIN((size_t)ed->goal_col - 1, len_next_line);
        ed->cursor.prev_disp_row = ed->cursor.disp_row;
        ed->cursor.disp_row++;
        ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorMoveEndOfNextWord (Editor *ed) {
    size_t prev_pos = ed->cursor.buffer_idx;
    editorMoveRight(ed);
    UnicodeChar c = getBufChar(ed->buf, ed->cursor.buffer_idx);
    size_t buf_len = getBufLength(ed->buf);
    // skip spaces
    if (isspaceUTF8(c)) {
        while(isspaceUTF8(c) && c != '\n' && ed->cursor.buffer_idx != buf_len) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        }
    }

    if (ispunctUTF8(c) && c != '_') {
        while (ispunctUTF8(c) && c != '_' && ed->cursor.buffer_idx != buf_len) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        }
    } else if (isalnumUTF8(c) || c == '_') {
        while ((isalnumUTF8(c) || c == '_') && ed->cursor.buffer_idx != buf_len) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        }
    }
    ed->cursor.prev_buffer_idx = prev_pos;
}

void editorMoveBegOfPrevWord(Editor *ed) {
    size_t prev_pos = ed->cursor.buffer_idx;
    editorMoveLeft(ed);
    UnicodeChar c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));

    // skip spaces
    if (isspaceUTF8(c)) {
        while (isspaceUTF8(c) && c != '\n' && ed->cursor.buffer_idx != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        } 
    }

    if (ispunctUTF8(c) && c != '_') {
        while (ispunctUTF8(c) && c != '_' && ed->cursor.buffer_idx != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        }
    } else if (isalnumUTF8(c) || c == '_'){
        while ((isalnumUTF8(c) || c == '_') && ed->cursor.buffer_idx != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        }
    }
    ed->cursor.prev_buffer_idx = prev_pos;
}

void editorInsert(Editor *ed, char *bytes) {
    ed->cursor.moved_last_frame = true;
    size_t grapheme_size, offset = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);

        insertUnicodeCharIntoBuf (ed->buf, ed->cursor.buffer_idx, grapheme, grapheme_size);
        if (grapheme == '\n') { // new line
            ed->line_count++;
        }
        editorMoveRight(ed);
    }
    ed->cursor.pos_anim_time = 0.0f;

}

void editorDeleteLeft(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    // TODO: simplify this (remove redundant code)
    if (ed->cursor.buffer_idx != 0) {
        if (getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx)) != '\n') {
            removeGraphemeBeforeGap(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
            ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
            ed->goal_col = ed->cursor.disp_col;
        } else {
            removeGraphemeBeforeGap(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
            ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
            ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
            ed->goal_col = ed->cursor.disp_col;
            ed->cursor.prev_disp_row = ed->cursor.disp_row;
            ed->cursor.disp_row --;
            ed->line_count --;
        }
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorDeleteRight(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    if (removeGraphemeAfterGap(ed->buf, ed->cursor.buffer_idx) == '\n') {
        ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
    }
}

void editorDeleteWordLeft(Editor *ed) {
    editorDeleteLeft(ed);
    char c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
    
    // skip spaces
    if (isspaceUTF8(c)) {
        while (isspaceUTF8(c) && c != '\n' && ed->cursor.buffer_idx != 0) {
            editorDeleteLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        } 
    }

    if (ispunctUTF8(c) && c != '_') {
        while (ispunctUTF8(c) && c != '_' && ed->cursor.buffer_idx != 0) {
            editorDeleteLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        }
    } else if (isalnumUTF8(c) || c == '_'){
        while ((isalnumUTF8(c) || c == '_') && ed->cursor.buffer_idx != 0) {
            editorDeleteLeft(ed);
            c = getBufChar(ed->buf, getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx));
        }
    }
}

void editorDeleteWordRight(Editor *ed) {
    char c = getBufChar(ed->buf, ed->cursor.buffer_idx);
    
    // We don't want to skip here - if there are spaces we want to delete those
    // and let the user choose to delete more.
    if (c == '\n') {
        editorDeleteRight(ed);
    } else if (isspaceUTF8(c) && c != '\n') {
        while (isspaceUTF8(c) && c != '\n') {
            editorDeleteRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        } 
    } else if (ispunctUTF8(c) && c != '_') {
        while (ispunctUTF8(c) && c != '_') {
            editorDeleteRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        }
    } else if (isalnumUTF8(c) || c == '_'){
        while (isalnumUTF8(c) || c == '_') {
            editorDeleteRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_idx);
        }
    }
}
