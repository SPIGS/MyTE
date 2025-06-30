#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "editor.h"
#include "buffer.h"
#include "context.h"
#include "cursor.h"
#include <grapheme.h>
#include <string.h>
#include "lexer.h"
#include "putils/log.h"
#include "putils/pmath.h"
#include "putils/unicode.h"
#include "putils/pstring.h"
#include "putils/file.h"

Editor *editorNew(Rect frame, f32 line_height) {
    Editor *ed = (Editor *)malloc(sizeof(Editor));
    ed->buf = gapBufferNew(INITIAL_BUFFER_SIZE);
    ed->goal_col = -1;
    ed->line_count = 1;

    ed->gutter = (Gutter) {
        .size = vec2(0.0, 0.0),
        .txt_pos = vec2(0.0, 0.0)
    };

    ed->text_pos = vec2(0.0, 0.0);

    ed->frame = frame;
    ed->scroll_pos = vec2(0.0,0.0);
    ed->target_scroll_pos = vec2(0.0,0.0);
    ed->line_height = line_height;

    Vector2 pos = vec2(0, 0);
    pos.x = (frame.x + (frame.w / 2.0));
    pos.y = (frame.x + (frame.h / 2.0));

    ed->cursor = cursorNew(pos);
    ed->cursor_speed = CURSOR_SPEED;
    ed->scroll_speed = SCROLL_SPEED;
    ed->scroll_mode = SCROLL_MODE_CURSOR;

    ed->unsaved = false;
    ed->path = NULL;

    lexerInit(&ed->lexer);
    ed->dirty = true;

    return ed;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);

    if (ed->path)
        stringFree(ed->path);

    free(ed);
}

static void calculateGutterWidth(Editor *ed, AppContext *ctx) {
    // Get info needed to draw the gutter
    i32 num_lines = (i32)ed->line_count;
    i32 digits = 1;
    while (num_lines /= 10)
        digits++;

    ed->gutter.padding = 3;
    if (digits >= 3) {
        ed->gutter.padding = digits + 1;
    }
    
    f32 gutter_text_x_offset = ctx->glyph_width;
    ed->gutter.txt_pos = vec2(ed->frame.x + gutter_text_x_offset, ed->frame.y + ed->frame.h - ed->line_height);
    Vector2 gutter_scroll_offset = vec2(0.0, ed->scroll_pos.y);
    ed->gutter.txt_pos = vec2Add(ed->gutter.txt_pos, gutter_scroll_offset);

    // Get the gutter width 
    ed->gutter.size.x = 0.0;
    string num = stringNew("");
    num = stringFmt(num, "%*d", ed->gutter.padding, ed->line_count);
    ed->gutter.size.x = getSizeOfText(ctx->font_collection, ctx->glyph_cache, ctx->atlas, num, 1.0);
    ed->gutter.size.x += gutter_text_x_offset * 3;
    stringFree(num);
}

void editorUpdate(Editor *ed, AppContext *ctx, f64 delta_time) {
    // Initial cursor position
    calculateGutterWidth(ed, ctx);
    Vector2 adj_cursor_pos = vec2(ed->text_pos.x + ed->gutter.size.x + ctx->glyph_width, ed->text_pos.y - ed->line_height - (ed->line_height * 0.1));
    
    // Vertical/horizontal adjustment from column/row offset
    //First, get the horizontal offset from the beginning of the line
    size_t cursor_idx = ed->cursor.buffer_idx;
    size_t begin_line = getBeginningOfLineCursor(ed->buf, cursor_idx);
    for (size_t i = 0; i < (cursor_idx - begin_line); i++) {
        UnicodeChar grapheme = getBufChar(ed->buf, begin_line + i);
        char *unpacked_grapheme = unpackUTF8(grapheme);
        GlyphTexture *glyph = (GlyphTexture *)hashmapGet(ctx->glyph_cache, unpacked_grapheme);

        if (glyph == NULL) {
            cacheGrapheme(ctx->font_collection, ctx->glyph_cache, ctx->atlas, unpacked_grapheme);
        }
        glyph = (GlyphTexture *)hashmapGet(ctx->glyph_cache, unpacked_grapheme);
        free(unpacked_grapheme);

        if (grapheme == '\t') {
            glyph = (GlyphTexture *)hashmapGet(ctx->glyph_cache, " ");
            adj_cursor_pos.x += (TAB_WIDTH * glyph->advance);
        } else {
            adj_cursor_pos.x += (glyph->advance);
        }
    } 

    // Vertical offset
    adj_cursor_pos.y -= ed->line_height * (ed->cursor.disp_row - 1);

    // Update the cursor
    adj_cursor_pos = vec2Add(adj_cursor_pos, ed->scroll_pos);
    cursorUpdate(&ed->cursor, adj_cursor_pos, delta_time);

    if ( ed->scroll_mode != SCROLL_MODE_MOUSE) {
        ed->target_scroll_pos = ed->scroll_pos;

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
    }

    // Get the horizontal scroll position
    ed->scroll_pos = vec2Lerp(ed->scroll_pos, ed->target_scroll_pos, (f32)delta_time  * 35.0f);

    f32 status_line_height = ed->line_height + 5.0;
    f32 h = ctx->screen_height - status_line_height;
    f32 y = status_line_height;
    ed->frame = rect(0, y, ctx->screen_width, h);
    ed->text_pos = vec2(ed->text_pos.x, ctx->screen_height);


    // Update the lexer
    if (ed->dirty) {
        lex(&ed->lexer, ed->buf);
        ed->dirty = false;
    }
}

void editorSetPath(Editor *ed, const char *path) {
    if (ed->path)
        stringFree(ed->path);

    ed->path = stringNew(path);
}

void editorLoadFile(Editor *ed, const char *path) {
    i32 check = checkPath(path);
    if (check == 0) {
        // Load file
        LOG_DEBUG("Loading file %s", path);
        char *bytes = readFile(path);
        editorInsert(ed, bytes);
        free(bytes);

        // Set the file path for the editor
        editorSetPath(ed, path);

        // Move cursor to beginning of file
        ed->cursor.buffer_idx = 0;
        ed->cursor.prev_buffer_idx = 0;
        ed->cursor.disp_row = 1;
        ed->cursor.prev_disp_row = 1;
        ed->cursor.disp_col = 1;
        ed->goal_col = -1;

        // Don't prompt for save
        ed->unsaved = false;
    } else if (check == -1) {
        LOG_DEBUG("File not found: \'%s\', loading a blank buffer", path);
        editorSetPath(ed, path);
    }
}

void editorSaveFile(Editor *ed) {
    assert(ed->path);
    LOG_DEBUG("Writing file %s", ed->path);
    UnicodeChar *u = getBufferString(ed->buf);
    size_t len = getBufLength(ed->buf);
    string bytes = unpackUTF8String(u, len);
    writeFile(ed->path, bytes);
    stringFree(bytes);
    free(u);
    ed->unsaved = false;
}

size_t getBegginingOfCursorLine(Editor *ed) {
    return getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_idx);
}

void editorMoveLeft(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;

    // Handle the selection, if there is one
    if (ed->cursor.selection_size != 0) {
        editorDeleteSelection(ed);
    }

    size_t grapheme_size, offset = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);

        insertUnicodeCharIntoBuf (ed->buf, ed->cursor.buffer_idx, grapheme, grapheme_size);
        editorMoveRight(ed);

        if (grapheme == '\n') { // new line
            ed->line_count++;
        }
    }
    ed->cursor.pos_anim_time = 0.0f;
    ed->unsaved = true;
    ed->dirty = true;
}

void editorDeleteLeft(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
        ed->unsaved = true;
        ed->dirty = true;
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorDeleteRight(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    UnicodeChar g = removeGraphemeAfterGap(ed->buf, ed->cursor.buffer_idx);
    if (g == '\n') {
        ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
    }

    if (g != 0) {
        ed->unsaved = true;
        ed->dirty = true;
    }
}

void editorDeleteWordLeft(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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
    ed->scroll_mode = SCROLL_MODE_CURSOR;
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

void editorScrollWithMouseWheel(Editor *ed, f32 yoffset) {
    ed->scroll_mode = SCROLL_MODE_MOUSE;
    ed->target_scroll_pos.y += (-1.0 * yoffset) * (ed->line_height * ed->scroll_speed);

    // clamp the max scroll
    if (ed->target_scroll_pos.y > (ed->line_height * (ed->line_count - 1.0))) {
        ed->target_scroll_pos.y = (ed->line_height * (ed->line_count - 1.0));
    } else if (ed->target_scroll_pos.y < 0) {
        ed->target_scroll_pos.y = 0;
    }
}

void moveCursorToMousePos(Editor *ed, AppContext *ctx, f64 mouse_x, f64 mouse_y) {
    ed->cursor.moved_last_frame = true;
    
    i32 row = MAX((i32)((ed->scroll_pos.y + mouse_y) / ed->line_height), 0);
    i32 max_row = MIN((i32)((ed->scroll_pos.y + ed->frame.h) / ed->line_height), (i32)ed->line_count - 1);
    row = MIN(max_row, row);

    // Move the cursor to the correct line
    size_t cur_row = 0;
    size_t i = 0;
    while ((i32)cur_row != row) {
        UnicodeChar uc = getBufChar(ed->buf, i);
        if (uc == '\n') {
            cur_row++;
        }
        i = getNextGraphemeCursor(ed->buf, i);
    }

    // Move the cursor to the correct column
    i32 max_col = getBufLineLength(ed->buf, i);
    i32 col = getBufColumn(ed->buf, i);
    f64 text_base = ed->gutter.size.x + (ctx->glyph_width * 1.5f);
    f64 cursor_x = text_base;
    f64 size_text = 0.0f;
    string line = stringNew("");
    while (cursor_x <= mouse_x) {
        UnicodeChar uc = getBufChar(ed->buf, col);
        char * upc = unpackUTF8(uc);
        line = stringCatStr(line, upc);
        size_text = getSizeOfText(ctx->font_collection, ctx->glyph_cache, ctx->atlas, line, 1.0);
        cursor_x = text_base + size_text;
        free(upc);
        col++;
    }
    stringFree(line);
    col = MIN(max_col, col);
    i += col;
    
    // if we moved past the end of the buffer, set it equal to the end of the buffer
    if (i >= getBufLength(ed->buf)) {
        i = getBufLength(ed->buf);
    }

    ed->cursor.prev_buffer_idx = ed->cursor.buffer_idx;
    ed->cursor.buffer_idx = i;
    ed->cursor.disp_col = getBufColumn(ed->buf, ed->cursor.buffer_idx) + 1;
    ed->cursor.disp_row = row + 1;
    ed->cursor.pos_anim_time = 0.0f;
    ed->goal_col = ed->cursor.disp_col;
}


void editorMakeSelection(Editor *ed) {
    if (ed->cursor.buffer_idx != ed->cursor.prev_buffer_idx) {
        i32 new_sel_size = (i32)ed->cursor.buffer_idx - (i32)ed->cursor.prev_buffer_idx;
        if (ed->cursor.selection_size == 0) {
            ed->cursor.beg_sel_scrn_pos = ed->cursor.target_screen_pos;
        }
        ed->cursor.selection_size += new_sel_size;
    }
    LOG_DEBUG("Selection size: %d", ed->cursor.selection_size);
}

void editorUnselectSelection(Editor *ed) {
    ed->cursor.selection_size = 0;
    LOG_DEBUG("Unselect selection");
}

static void deleteSelectionLeft(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    ed->cursor.moved_last_frame = true;
    size_t i = ed->cursor.buffer_idx;
    size_t sel_beg = ed->cursor.buffer_idx - ed->cursor.selection_size;
    while(i > sel_beg) {
        editorDeleteLeft(ed);
        i--;
    }
    editorUnselectSelection(ed);
}

static void deleteSelectionRight(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    ed->cursor.moved_last_frame = true;
    size_t i = ed->cursor.buffer_idx;
    size_t sel_end = ed->cursor.buffer_idx - ed->cursor.selection_size;
    while(i < sel_end) {
        editorDeleteRight(ed);
        i++;
    }
    editorUnselectSelection(ed);
}

void editorDeleteSelection(Editor *ed) {
    if (ed->cursor.selection_size > 0) {
        deleteSelectionLeft(ed);
    } else if (ed->cursor.selection_size < 0) {
        deleteSelectionRight(ed);
    }
}

string editorGetSelectionBytes(Editor *ed) {
    if (ed->cursor.selection_size > 0) {
        size_t buf_pos = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
        string sel_bytes = stringNew("");
        for (i32 i = 0; i < ed->cursor.selection_size; i++) {
            UnicodeChar c = getBufChar(ed->buf, buf_pos);
            char *char_bytes = unpackUTF8(c);
            string new_sel_bytes = stringNew(char_bytes);
            new_sel_bytes = stringCatStr(new_sel_bytes, sel_bytes);
            sel_bytes = stringDup(new_sel_bytes);
            free(char_bytes);
            buf_pos = getPrevGraphemeCursor(ed->buf, buf_pos);
        }
        return sel_bytes;
    } else if (ed->cursor.selection_size < 0) {
        size_t buf_pos = ed->cursor.buffer_idx;
        string sel_bytes = stringNew("");
        for (i32 i = 0; i < abs(ed->cursor.selection_size); i++) {
            UnicodeChar c = getBufChar(ed->buf, buf_pos);
            char *char_bytes = unpackUTF8(c);
            sel_bytes = stringCatStr(sel_bytes, char_bytes);
            free(char_bytes);
            buf_pos = getNextGraphemeCursor(ed->buf, buf_pos);
        }
        return sel_bytes;
    }
    return NULL;
}
