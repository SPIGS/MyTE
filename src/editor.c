#include "editor.h"
#include <stdio.h>

Gutter gutterInit(vec2 screen_pos, f32 glyph_adv) {
    return (Gutter) {
        .screen_pos = screen_pos,
        .gutter_width = glyph_adv * 4.0,
        .digits = 1
    };
}

static void calculateGutterWidth(Editor *ed, AppContext *ctx) {
    // calculate the new gutter width;
    ed->gutter.digits = 1;
	i32 num_lines = (i32)ed->line_count;
	while (num_lines /= 10)
		ed->gutter.digits++;
    
    f32 gutter_padding = MAX(ed->gutter.digits + 2, 4);
	ed->gutter.gutter_width = ctx->glyph_adv * gutter_padding;

    //update positions of everything
    f32 text_offset_x = ed->gutter.gutter_width + (ctx->glyph_adv * 3);
    ed->text_pos = vec2_init(text_offset_x, ed->frame.y + ed->frame.h);
}

void editorInit(Editor *ed, rect frame, AppContext *ctx, const char *cur_dir) {
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->gutter = gutterInit(vec2_init(frame.x, frame.y), ctx->glyph_adv);
    ed->cursor = cursorInit(vec2_init(frame.x + ed->gutter.gutter_width + (ctx->glyph_adv * 3), frame.h), 0.5);
    ed->goal_column = -1;
    ed->frame = rect_init(frame.x, frame.y, frame.w, frame.h);
    ed->text_pos = vec2_init(frame.x + ed->gutter.gutter_width + (ctx->glyph_adv * 3), frame.y + frame.h);
    ed->scroll_pos = vec2_init(0,0);
    ed->target_scroll_pos = vec2_init(0,0);
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    ed->line_count = 1;
    ed->file_path = NULL;
    lexerInit(&ed->lexer);
    ed->dirty = true;
    ed->tab_stop = 4;
    ed->cursor_speed = 3.5;
    ed->scroll_speed = 1;
    ed->scroll_stop_top = 6;
    ed->scroll_stop_bottom = 2;

    ed->mode = EDITOR_MODE_NORMAL;
    fileBrowserInit(&ed->browser, vec2_init(frame.x, frame.h), cur_dir);
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
    lexerDestroy(&ed->lexer);
    fileBrowserDestroy(&ed->browser);
}

void editorLoadConfig(Editor *ed, Config *config) {
    ed->tab_stop = config->tab_stop;
    ed->cursor_speed = config->cursor_speed;
    ed->scroll_speed = config->scroll_speed;
    ed->dirty = true;
}

void editorChangeMode(Editor *ed, AppContext *ctx, EditorMode new_mode) {
    resetAnimTime(&ed->cursor);
    resetAnimTime(&ed->browser.cursor);

    switch (new_mode) {
    case EDITOR_MODE_OPEN:
        LOG_INFO("Load paths", "");
        ed->browser.cursor.screen_pos = ed->cursor.screen_pos;
        ed->browser.cursor.width = ed->cursor.width;
        getPaths(&ed->browser);
        break;

    case EDITOR_MODE_SAVE:
        LOG_INFO("save mode", "");
        ed->sd = dialogInit(ed->cursor.screen_pos, ctx->line_height, ctx->glyph_adv);
        ed->sd.cursor.moved_last_frame = true;
        break;
    
    default:
        if (ed->mode == EDITOR_MODE_SAVE && new_mode != EDITOR_MODE_SAVE) {
            dialogDestroy(&ed->sd);
        }
        
        if (ed->mode == EDITOR_MODE_SAVE) {
            ed->cursor.screen_pos = ed->sd.cursor.screen_pos;
            ed->cursor.moved_last_frame = true;
        } else if (ed->mode == EDITOR_MODE_OPEN){
            ed->cursor.screen_pos = ed->browser.cursor.screen_pos;
            ed->cursor.moved_last_frame = true;
        }
        break;
    }
    ed->mode = new_mode;
}

void editorMoveLeft(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    if (ed->cursor.buffer_pos == 0) {
        ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
        return;
    }

    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);

    // If we moved to the prev line
    if (getBufChar(ed->buf, ed->cursor.buffer_pos) == '\n') {
        ed->cursor.disp_row--;  
    }

    ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
    ed->cursor.pos_anim_time = 0.0f;
    ed->goal_column = ed->cursor.disp_column;
}

void editorMoveRight(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    if (ed->cursor.buffer_pos == getBufLength(ed->buf)) {
        ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
        return;
    }

    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    ed->cursor.buffer_pos = getNextCharCursor(ed->buf, ed->cursor.buffer_pos);

    // If we moved to the next line
    if (ed->cursor.buffer_pos == getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_pos)) {
        ed->cursor.disp_row++;
    }

    ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
    ed->cursor.pos_anim_time = 0.0f;
    ed->goal_column = ed->cursor.disp_column;
}

void editorMoveUp(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    if (ed->goal_column == -1) {
        ed->goal_column = ed->cursor.disp_column;
    }
    
    size_t beg_prev_line = getBeginningOfPrevLineCursor(ed->buf, ed->cursor.buffer_pos);
    size_t beg_line = getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_pos);
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;

    if (beg_line == 0) {
        ed->cursor.buffer_pos = beg_line;
        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        ed->goal_column = ed->cursor.disp_column;
    } else {
        size_t len_prev_line = getBufLineLength(ed->buf, beg_prev_line);
        ed->cursor.buffer_pos = beg_prev_line + MIN((size_t)ed->goal_column - 1, len_prev_line);
        ed->cursor.disp_row--;
        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorMoveDown(Editor *ed) {
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    if (ed->goal_column == -1) {
        ed->goal_column = ed->cursor.disp_column;
    }

    size_t beg_next_line = getBeginningOfNextLineCursor(ed->buf, ed->cursor.buffer_pos);
    size_t end_line = getEndOfLineCursor(ed->buf, ed->cursor.buffer_pos);
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;

    if (end_line == getBufLength(ed->buf)) {
        ed->cursor.buffer_pos = beg_next_line;
        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        ed->goal_column = ed->cursor.disp_column;
    } else {
        size_t len_next_line = getBufLineLength(ed->buf, beg_next_line);
        ed->cursor.buffer_pos = beg_next_line + MIN((size_t)ed->goal_column - 1, len_next_line);
        ed->cursor.disp_row++;
        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
    }
    ed->cursor.pos_anim_time = 0.0f;
}

void editorInsertCharacter(Editor *ed, char character, bool move_cursor_forward) {
    if (ed->cursor.selection_size != 0) {
        editorDeleteSelection(ed);
    } else {
        editorUnselectSelection(ed);
    }
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    insertCharIntoBuf(ed->buf, ed->cursor.buffer_pos, character);
    if (character == '\n') {
        ed->line_count ++;
    }
    
    if (move_cursor_forward) {
        editorMoveRight(ed);
    }
    ed->dirty = true;
}

void editorDeleteCharLeft(Editor *ed) {
    editorUnselectSelection(ed);
    ed->cursor.moved_last_frame = true;
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    if (ed->cursor.buffer_pos != 0) {
        if (getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos)) != '\n') {
            removeCharBeforeGap (ed->buf, ed->cursor.buffer_pos);
            ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
            ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
            ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
            ed->goal_column = ed->cursor.disp_column;
            ed->cursor.pos_anim_time = 0.0f;
            ed->dirty = true;
        } else {
            removeCharBeforeGap (ed->buf, ed->cursor.buffer_pos);
            ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
            ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
            ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
            ed->goal_column = ed->cursor.disp_column;
            ed->cursor.disp_row--;
            ed->line_count--;
            ed->cursor.pos_anim_time = 0.0f;
            ed->dirty = true;
        }
    }
}

void editorDeleteCharRight(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        editorUnselectSelection(ed);
        ed->cursor.moved_last_frame = true;
        ed->scroll_mode = SCROLL_MODE_CURSOR;
        if (removeCharAfterGap (ed->buf, ed->cursor.buffer_pos) == '\n') {
            ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
        }
        ed->dirty = true;
    }
}

void editorUpdate(Editor *ed, AppContext *ctx, f64 delta_time) {
    //Initial cursor position
    vec2 adj_cursor_pos = vec2_init(ed->text_pos.x, ed->text_pos.y - ctx->line_height - ctx->descender);
    if (ed->mode == EDITOR_MODE_NORMAL) {
        // Vertical/horizontal adjustment from column/row offset
        adj_cursor_pos.x += ctx->glyph_adv * (ed->cursor.disp_column - 1);
        adj_cursor_pos.y -= ctx->line_height * (ed->cursor.disp_row - 1);

        // offet from scroll position
        adj_cursor_pos = vec2_add(adj_cursor_pos, ed->scroll_pos);
        cursorUpdate(&ed->cursor, adj_cursor_pos, delta_time);
    } else if (ed->mode == EDITOR_MODE_OPEN) {
        // Update the selection highlight
        f32 target_sel_w = (strlen(ed->browser.items[ed->browser.selection].name_ext) + (ed->browser.items[ed->browser.selection].is_dir ? 1 : 0)) * ctx->glyph_adv;
        ed->browser.cursor.target_width = (target_sel_w) + 5;
        ed->browser.cursor.prev_width = ed->browser.cursor.width;
        
        adj_cursor_pos = vec2_init(adj_cursor_pos.x, adj_cursor_pos.y - (ctx->line_height * (ed->browser.selection)));
        cursorUpdate(&ed->browser.cursor, adj_cursor_pos, delta_time);
    } else if (ed->mode == EDITOR_MODE_SAVE) {
        dialogUpdate(&ed->sd, ctx, delta_time);
    }

    if (ed->scroll_mode != SCROLL_MODE_MOUSE) {
        ed->target_scroll_pos = vec2_init(ed->scroll_pos.x, ed->scroll_pos.y);

        f32 bottom_scroll_bound = ed->frame.y + (ed->scroll_stop_bottom * ctx->line_height);
        f32 top_scroll_bound = ed->frame.y + ed->frame.h - (ed->scroll_stop_top * ctx->line_height);
        f32 bottom_line_y = ctx->line_height * ed->line_count;
        
        if (ed->cursor.target_screen_pos.y <= bottom_scroll_bound && ed->scroll_pos.y < bottom_line_y) {
            ed->target_scroll_pos.y += ctx->line_height;
        } else if (ed->cursor.target_screen_pos.y >= top_scroll_bound && ed->scroll_pos.y > 0.0) {
            ed->target_scroll_pos.y -= ctx->line_height;
        }
    }

    ed->scroll_pos = vec2_lerp(ed->scroll_pos, ed->target_scroll_pos, (f32)delta_time  * 35.0f);

    // Update frame
    f32 STATUS_LINE_HEIGHT = ctx->line_height;
    ed->frame = rect_init(0,0 + STATUS_LINE_HEIGHT, ctx->screen_width, ctx->screen_height - STATUS_LINE_HEIGHT);
    ed->text_pos = vec2_init(ed->text_pos.x, 0 + ctx->screen_height);
    calculateGutterWidth(ed, ctx);

    // Update the lexer
    if (ed->dirty) {
        char *data = getBufString(ed->buf);
        lex(&ed->lexer, data);
        free(data);
        ed->dirty = false;
    }
}

void editorLoadFile(Editor *ed, AppContext *ctx, const char *file_path) {
    // Get file info
    const char *file_ext = getFileExtFromPath(file_path);
    char *file_name = getFileNameFromPath(file_path);
    FileType ftype = getFileType(file_name, file_ext);
    
    // Update lexer info
    lexerUpdateFileType(&ed->lexer, ftype);
    free(file_name);

     // If there's stuff in the editor already, we don't care for now just over write it
    FILE *f = fopen(file_path, "r");
    
    if (f == NULL) {
        LOG_INFO("No file named \'%s\', opening an empty file", file_path);
    } else {
        // Load the file into the editor
        #define CHUNK 1024
        char read_buf[CHUNK];
        size_t nread;

        while ((nread = fread(read_buf, 1, sizeof(read_buf), f)) > 0) {
            for (size_t i = 0; i < nread; i++) {
                if (read_buf[i] == '\t') {
                    for (size_t i = 0; i < (size_t)ed->tab_stop; i++) {
                        editorInsertCharacter(ed, ' ', true);
                    }
                } else {
                    editorInsertCharacter(ed, read_buf[i], true);
                }
            }
        }
        fclose(f);
    }

    // move the cursor position back to the start of the file/ update some parameters
    ed->cursor.buffer_pos = 0;
    ed->cursor.prev_buffer_pos = 0;
    ed->cursor.disp_row = 1;
    ed->cursor.disp_column = 1;
    ed->goal_column = -1;
    ed->file_path = file_path;
    ed->dirty = true;
    calculateGutterWidth(ed, ctx);
}

void editorWriteFile(Editor *ed) {
    if (!ed->file_path) {
        LOG_ERROR("Writing a blank file to disk is not supported!", "");
        return;
    }

    FILE *f = fopen(ed->file_path, "w");
    if (f == NULL) {
        LOG_ERROR("Couldn't open file with write access: ", ed->file_path);
        return;
    }

    if (fputs(getBufString(ed->buf), f) == EOF) {
        LOG_ERROR("Couldn't write to file: ", ed->file_path);
        fclose(f);
        return;
    }
    fclose(f);

    LOG_INFO("Wrote to disk: %s", ed->file_path);
}

static void editorClearBuffer(Editor *ed) {
    gapBufferDestroy(ed->buf);
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorInit(vec2_init(ed->frame.x, ed->frame.h),  1.0);
    ed->goal_column = -1;
    ed->scroll_pos = vec2_init(0,0);
    ed->line_count = 1;
    ed->dirty = true;
}

void editorMoveEndOfNextWord(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    size_t prev_pos = ed->cursor.buffer_pos;
    editorMoveRight(ed);
    char c = getBufChar(ed->buf, ed->cursor.buffer_pos);
    
    // skip spaces
    if (isspace(c)) {
        while (isspace(c) && c != '\n' && ed->cursor.buffer_pos != getBufLength(ed->buf)) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        } 
    }

    if (ispunct(c) && c != '_') {
        while (ispunct(c) && c != '_' && ed->cursor.buffer_pos != getBufLength(ed->buf)) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        }
    } else if (isalnum(c) || c == '_'){
        while ((isalnum(c) || c == '_') && ed->cursor.buffer_pos != getBufLength(ed->buf)) {
            editorMoveRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        }
    }
    ed->cursor.prev_buffer_pos = prev_pos;
}

void editorMoveBegOfPrevWord(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    size_t prev_pos = ed->cursor.buffer_pos;
    editorMoveLeft(ed);
    char c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
    
    // skip spaces
    if (isspace(c)) {
        while (isspace(c) && c != '\n' && ed->cursor.buffer_pos != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        } 
    }

    if (ispunct(c) && c != '_') {
        while (ispunct(c) && c != '_' && ed->cursor.buffer_pos != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        }
    } else if (isalnum(c) || c == '_'){
        while ((isalnum(c) || c == '_') && ed->cursor.buffer_pos != 0) {
            editorMoveLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        }
    }
    ed->cursor.prev_buffer_pos = prev_pos;
}

void editorDeleteWordLeft(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    editorDeleteCharLeft(ed);
    char c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
    
    // skip spaces
    if (isspace(c)) {
        while (isspace(c) && c != '\n' && ed->cursor.buffer_pos != 0) {
            editorDeleteCharLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        } 
    }

    if (ispunct(c) && c != '_') {
        while (ispunct(c) && c != '_' && ed->cursor.buffer_pos != 0) {
            editorDeleteCharLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        }
    } else if (isalnum(c) || c == '_'){
        while ((isalnum(c) || c == '_') && ed->cursor.buffer_pos != 0) {
            editorDeleteCharLeft(ed);
            c = getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos));
        }
    }
}

void editorDeleteWordRight(Editor *ed) {
    ed->scroll_mode = SCROLL_MODE_CURSOR;
    char c = getBufChar(ed->buf, ed->cursor.buffer_pos);
    
    // We don't want to skip here - if there are spaces we want to delete those
    // and let the user choose to delete more.
    if (c == '\n') {
        editorDeleteCharRight(ed);
    } else if (isspace(c) && c != '\n') {
        while (isspace(c) && c != '\n') {
            editorDeleteCharRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        } 
    } else if (ispunct(c) && c != '_') {
        while (ispunct(c) && c != '_') {
            editorDeleteCharRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        }
    } else if (isalnum(c) || c == '_'){
        while (isalnum(c) || c == '_') {
            editorDeleteCharRight(ed);
            c = getBufChar(ed->buf, ed->cursor.buffer_pos);
        }
    }
}

void moveCursorToMousePos(Editor *ed, AppContext *ctx, vec2 screen_pos) {
    ed->cursor.moved_last_frame = true;
    i32 row = MAX((i32)((ed->scroll_pos.y + screen_pos.y) / ctx->line_height), 0);
    i32 col = MAX((i32)((screen_pos.x - ed->text_pos.x + (ctx->glyph_adv * 0.5)) / ctx->glyph_adv), 0);

    // clamp to maximum line
    i32 max_row = MIN((i32)((ed->scroll_pos.y + ed->frame.h) / ctx->line_height), (i32)ed->line_count - 1);
    row = MIN(max_row, row);

    // move the cursor to the correct line
    size_t cur_row = 0;
    size_t i = 0;
    while ((i32)cur_row != row) {
        char c = getBufChar(ed->buf, i);
        if (c == '\n') {
            cur_row++;
        }
        i = getNextCharCursor(ed->buf, i);
    }

    // clamp to line length
    i32 max_col = getBufLineLength(ed->buf, i);
    col = MIN(max_col, col);

    //move the cursor to the correct column
    while (col > 0) {
        i = getNextCharCursor(ed->buf, i);
        col--;
    }

    // if we moved past the end of the buffer, set it equal to the end of the buffer
    if (i >= getBufLength(ed->buf)) {
        i = getBufLength(ed->buf);
    }

    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    ed->cursor.buffer_pos = i;
    ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
    ed->cursor.disp_row = row + 1;
    ed->cursor.pos_anim_time = 0.0f;
    ed->goal_column = ed->cursor.disp_column;
}

void scrollWithMouseWheel(Editor *ed, AppContext *ctx, f32 yoffset) {
    ed->scroll_mode = SCROLL_MODE_MOUSE;
    ed->target_scroll_pos.y += (-1.0 * yoffset) * (ctx->line_height * ed->scroll_speed);

    // clamp the max scroll
    if (ed->target_scroll_pos.y > (ctx->line_height * (ed->line_count - 1.0))) {
        ed->target_scroll_pos.y = (ctx->line_height * (ed->line_count - 1.0));
    } else if (ed->target_scroll_pos.y < 0) {
        ed->target_scroll_pos.y = 0;
    }
}

void editorMakeSelection (Editor *ed) {
    if (ed->cursor.buffer_pos != ed->cursor.prev_buffer_pos) {
        
        i32 new_selection_size = (i32)ed->cursor.buffer_pos - (i32)ed->cursor.prev_buffer_pos;
        if (ed->cursor.selection_size == 0) {
            ed->cursor.screen_pos_beg_selection = ed->cursor.target_screen_pos;
        }
        ed->cursor.selection_size += new_selection_size;
    }
}

void editorUnselectSelection(Editor *ed) {
    ed->cursor.selection_size = 0;
}

static void deleteSelectionLeft(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        ed->scroll_mode = SCROLL_MODE_CURSOR;
        ed->cursor.moved_last_frame = true;
        size_t i = ed->cursor.buffer_pos;
        size_t selection_beg = ed->cursor.buffer_pos - ed->cursor.selection_size;
        while (i > selection_beg) {
            editorDeleteCharLeft(ed);
            i--;
        }
        editorUnselectSelection(ed);
    }
}

static void deleteSelectionRight(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        ed->scroll_mode = SCROLL_MODE_CURSOR;
        ed->cursor.moved_last_frame = true;
        size_t i = ed->cursor.buffer_pos;
        size_t selection_end = ed->cursor.buffer_pos - ed->cursor.selection_size;
        while (i < selection_end) {
            editorDeleteCharRight(ed);
            i++;
        }
        editorUnselectSelection(ed);
    }
}

void editorDeleteSelection(Editor *ed) {
    if (ed->cursor.selection_size > 0) {
        deleteSelectionLeft(ed);
    } else if (ed->cursor.selection_size < 0) {
        deleteSelectionRight(ed);
    }
}