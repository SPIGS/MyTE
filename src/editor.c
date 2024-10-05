#include "editor.h"
#include <stdio.h>

Cursor cursorInit(vec2 init_screen_pos) {
    return (Cursor) { 
        .buffer_pos = 0, 
        .prev_buffer_pos = 0, 
        .screen_pos = init_screen_pos, 
        .prev_screen_pos = init_screen_pos, 
        .target_screen_pos = init_screen_pos,
        .anim_time = 0.0f,
        .disp_column = 1,
        .disp_row = 1
    };
}

void editorInit(Editor *ed, rect frame, f32 line_height, f32 glyph_adv, f32 descender, const char *cur_dir) {
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorInit(vec2_init(frame.x, frame.h));
    ed->goal_column = -1;
    ed->frame = rect_init(frame.x, frame.y, frame.w, frame.h);
    ed->text_pos = vec2_init(frame.x, frame.y + frame.h);
    ed->scroll_pos = vec2_init(0,0);
    ed->target_scroll_pos = vec2_init(0,0);
    ed->line_count = 1;
    ed->line_height = line_height;
    ed->glyph_adv = glyph_adv;
    ed->descender = descender;
    ed->file_path = NULL;
    lexerInit(&ed->lexer);
    ed->dirty = true;
    ed->tab_stop = 4;
    ed->cursor_speed = 3.5;
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
    ed->dirty = true;
}

void editorChangeMode(Editor *ed, EditorMode new_mode) {
    ed->cursor.anim_time = 0.0f;
    ed->browser.anim_time = 0.0f;
    switch (new_mode)
    {
    case EDITOR_MODE_OPEN:
        LOG_INFO("Load paths", "");
        ed->browser.sel_screen_pos = ed->cursor.screen_pos;
        ed->browser.sel_size = vec2_init(3, ed->browser.sel_size.y);
        break;
    
    default:
        ed->cursor.screen_pos = ed->browser.sel_screen_pos;
        break;
    }
    ed->mode = new_mode;
}

void moveCursorLeft(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (ed->cursor.buffer_pos == 0) {
            return;
        }

        ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
        ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);

        // If we moved to the prev line
        if (getBufChar(ed->buf, ed->cursor.buffer_pos) == '\n') {
           ed->cursor.disp_row--;  
        }

        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        ed->cursor.anim_time = 0.0f;
        ed->goal_column = ed->cursor.disp_column;
    } 
}

void moveCursorRight(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (ed->cursor.buffer_pos == getBufLength(ed->buf)) {
            return;
        }

        ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
        ed->cursor.buffer_pos = getNextCharCursor(ed->buf, ed->cursor.buffer_pos);

        // If we moved to the next line
        if (ed->cursor.buffer_pos == getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_pos)) {
            ed->cursor.disp_row++;
        }

        ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        ed->cursor.anim_time = 0.0f;
        ed->goal_column = ed->cursor.disp_column;
    }
}

void moveCursorUp(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
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
            ed->cursor.buffer_pos = beg_prev_line + MIN(ed->goal_column - 1, len_prev_line);
            ed->cursor.disp_row--;
            ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        }
        ed->cursor.anim_time = 0.0f;
    } else {
        decrementSelection(&ed->browser);
    }
}

void moveCursorDown(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
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
            ed->cursor.buffer_pos = beg_next_line + MIN(ed->goal_column - 1, len_next_line);
            ed->cursor.disp_row++;
            ed->cursor.disp_column = getBufColumn(ed->buf, ed->cursor.buffer_pos) + 1;
        }
        ed->cursor.anim_time = 0.0f;
    } else {
        incrementSelection(&ed->browser);
    }
}

void insertCharacter(Editor *ed, char character, bool move_cursor_forward) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        insertCharIntoBuf(ed->buf, ed->cursor.buffer_pos, character);
        if (character == '\n') {
            ed->line_count ++;
        }
        
        if (move_cursor_forward) {
            moveCursorRight(ed);
        }
        ed->dirty = true;
    }
}

void deleteCharacterLeft(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (ed->cursor.buffer_pos != 0) {
            removeCharBeforeGap (ed->buf, ed->cursor.buffer_pos);
            ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;\

            if (getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos)) != '\n') {
                ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
                ed->cursor.disp_column--;
                ed->goal_column = ed->cursor.disp_column;
                ed->cursor.anim_time = 0.0f;
                ed->dirty = true;
            } else {
                ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
                ed->cursor.disp_column = getBufColumn(ed->buf, getEndOfPrevLineCursor(ed->buf, ed->cursor.buffer_pos)) + 1;
                ed->goal_column = ed->cursor.disp_column;
                ed->cursor.disp_row--;
                ed->line_count--;
                ed->cursor.anim_time = 0.0f;
                ed->dirty = true;
            }
        }
    }
}

void deleteCharacterRight(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (removeCharAfterGap (ed->buf, ed->cursor.buffer_pos) == '\n') {
            ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
        }
        ed->dirty = true;
    }
}

void setCursorTargetScreenPos(Editor *ed, vec2 new_target) {
    ed->cursor.target_screen_pos = new_target;
    ed->cursor.prev_screen_pos = ed->cursor.screen_pos;
    ed->browser.sel_target_screen_pos = new_target;
    ed->browser.sel_prev_screen_pos = ed->browser.sel_screen_pos;
}

void lerpCursorScreenPos(Editor *ed) {
    ed->cursor.screen_pos = vec2_ease_out(ed->cursor.prev_screen_pos, ed->cursor.target_screen_pos, ed->cursor.anim_time);
    ed->browser.sel_screen_pos = vec2_ease_out(ed->browser.sel_prev_screen_pos, ed->browser.sel_target_screen_pos, ed->browser.anim_time);
}

char *getContents(Editor *ed) {
    return getBufString(ed->buf);
}

void editorUpdate(Editor *ed, f32 screen_width, f32 screen_height, ColorTheme theme, f64 delta_time) {

    //Update cursor position
    //Initial cursor position
    f32 PADDING = 30.0f;
    vec2 adj_cursor_pos = vec2_init(((ed->glyph_adv * 3) + PADDING), ed->text_pos.y - ed->line_height - ed->descender);

    if (ed->mode == EDITOR_MODE_NORMAL) {
        // Vertical/horizontal adjustment from column/row offset
        adj_cursor_pos.x += ed->glyph_adv * (ed->cursor.disp_column - 1);
        adj_cursor_pos.y -= ed->line_height * (ed->cursor.disp_row - 1);

        // offet from scroll position
        adj_cursor_pos = vec2_add(adj_cursor_pos, ed->scroll_pos);
        setCursorTargetScreenPos(ed, vec2_init(adj_cursor_pos.x, adj_cursor_pos.y));

        // increment the animation timer
        //ed->cursor.anim_time += (f32)delta_time * ed->cursor_speed;
        ed->cursor.anim_time += (f32)delta_time;
        if (ed->cursor.anim_time >= 1.0f) {
            ed->cursor.anim_time = 1.0f;
        }

        lerpCursorScreenPos(ed);
    } else if (ed->mode == EDITOR_MODE_OPEN) {
        // Render the selection highlight
        getPaths(&ed->browser);
		setCursorTargetScreenPos(ed, vec2_init(adj_cursor_pos.x, adj_cursor_pos.y - (ed->line_height * (ed->browser.selection))));
		f32 target_sel_w = (strlen(ed->browser.items[ed->browser.selection].name_ext) + (ed->browser.items[ed->browser.selection].is_dir ? 1 : 0)) * ed->glyph_adv;
        ed->browser.sel_target_size = vec2_init((target_sel_w) + 5, ed->line_height);

        ed->browser.sel_prev_size = ed->browser.sel_size;

        //ed->browser.anim_time += (f32)delta_time * ed->cursor_speed;
        ed->browser.anim_time += (f32)delta_time;
		if (ed->browser.anim_time >= 1.0f)
			ed->browser.anim_time = 1.0f;

		lerpCursorScreenPos(ed);
		ed->browser.sel_size = vec2_ease_out(ed->browser.sel_prev_size, ed->browser.sel_target_size, ed->browser.anim_time);
    }

    ed->target_scroll_pos = vec2_init(ed->scroll_pos.x, ed->scroll_pos.y);
    
    f32 SCROLL_UP_LINE_FACTOR = 6;
    f32 SCROLL_DOWN_LINE_FACTOR = 2;

    // If  the cursor is moving down
    if (ed->cursor.target_screen_pos.y <= (ed->frame.y + (SCROLL_DOWN_LINE_FACTOR * ed->line_height)) && ed->scroll_pos.y < (ed->line_height * ed->line_count)) {
        //ed->cursor.target_screen_pos.y = ed->frame.y;
        ed->target_scroll_pos.y += ed->line_height;
    // If the cursor is moving up
    } else if (ed->cursor.target_screen_pos.y >= (ed->frame.y + ed->frame.h - (SCROLL_UP_LINE_FACTOR * ed->line_height)) && ed->scroll_pos.y > 0.0) {
        //ed->cursor.target_screen_pos.y = (ed->frame.y + ed->frame.h) - ed->line_height;
        ed->target_scroll_pos.y -= ed->line_height;
    }

    ed->scroll_pos = vec2_lerp(ed->scroll_pos, ed->target_scroll_pos, (f32)delta_time  * 35.0f);

    // Update frame
    f32 STATUS_LINE_HEIGHT = ed->line_height;
    ed->frame = rect_init(0,0 + STATUS_LINE_HEIGHT, screen_width, screen_height - STATUS_LINE_HEIGHT);
    ed->text_pos = vec2_init(0, 0 + screen_height);

    // Update the lexer
    if (ed->dirty) {
        char *data = getBufString(ed->buf);
        free(ed->lexer.tokens);
        lex(&ed->lexer, data, theme);
        ed->dirty = false;
    }
}

void loadFromFile(Editor *ed, const char *file_path) {
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
        fprintf(stderr, "ERROR: could not open file '%s'\n", file_path);
        exit(1);
    }

    #define CHUNK 1024
    char read_buf[CHUNK];
    size_t nread;

    while ((nread = fread(read_buf, 1, sizeof(read_buf), f)) > 0) {
        for (size_t i = 0; i < nread; i++) {
            if (read_buf[i] == '\t') {
                for (size_t i = 0; i < (size_t)ed->tab_stop; i++) {
                    insertCharacter(ed, ' ', true);
                }
            } else {
                insertCharacter(ed, read_buf[i], true);
            }
        }
    }
    fclose(f);

    // move the cursor position back to the start of the file
    ed->cursor.buffer_pos = 0;
    ed->cursor.prev_buffer_pos = 0;
    ed->cursor.disp_row = 1;
    ed->cursor.disp_column = 1;
    ed->goal_column = -1;
    ed->file_path = file_path;
    ed->dirty = true;
}

void writeToFile(Editor *ed) {
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

    LOG_INFO("Wrote to disk: ", ed->file_path);
}

void clearBuffer(Editor *ed) {
    gapBufferDestroy(ed->buf);
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorInit(vec2_init(ed->frame.x, ed->frame.h));
    ed->goal_column = -1;
    ed->scroll_pos = vec2_init(0,0);
    ed->line_count = 1;
    ed->dirty = true;
}

// TODO: These arent perfect but they are ok for now
void moveCursorWordForward(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (ed->cursor.buffer_pos >= ed->buf->end)
            return;

        size_t new_cursor_pos = ed->cursor.buffer_pos;
        char cur_char = getBufChar(ed->buf, new_cursor_pos);
        if (isspace(cur_char)) {
            while (!isalnum(cur_char) && cur_char != '\0') {
                new_cursor_pos = getNextCharCursor(ed->buf, new_cursor_pos);
                moveCursorRight(ed);
                cur_char = getBufChar(ed->buf, new_cursor_pos);
            }
        } else {
            while (!isalnum(cur_char) && !isspace(cur_char) && cur_char != '\0') {
                new_cursor_pos = getNextCharCursor(ed->buf, new_cursor_pos);
                moveCursorRight(ed);
                cur_char = getBufChar(ed->buf, new_cursor_pos);
            }
        }
        
        while (isalnum(getBufChar(ed->buf, new_cursor_pos))) {    
            new_cursor_pos = getNextCharCursor(ed->buf, new_cursor_pos);
            moveCursorRight(ed);
        }
    } else {
        
    }

    
}

void moveCursorWordBackward(Editor *ed) {
    if (ed->mode != EDITOR_MODE_OPEN) {
        if (ed->cursor.buffer_pos <= 0)
            return;

        size_t new_cursor_pos = ed->cursor.buffer_pos;
        char prev_char = getBufChar(ed->buf, getPrevCharCursor(ed->buf, new_cursor_pos));
        if (isspace(prev_char)) {
            while (!isalnum(prev_char) && new_cursor_pos != 0) {
                new_cursor_pos = getPrevCharCursor(ed->buf, new_cursor_pos);
                moveCursorLeft(ed);
                prev_char = getBufChar(ed->buf, getPrevCharCursor(ed->buf, new_cursor_pos));
            }
        } else {
            while (!isalnum(prev_char) && !isspace(prev_char) && new_cursor_pos != 0) {
                new_cursor_pos = getPrevCharCursor(ed->buf, new_cursor_pos);
                moveCursorLeft(ed);
                prev_char = getBufChar(ed->buf, getPrevCharCursor(ed->buf, new_cursor_pos));
            }
        }
        
        while (isalnum(getBufChar(ed->buf, getPrevCharCursor(ed->buf, new_cursor_pos)))) {    
            new_cursor_pos = getPrevCharCursor(ed->buf, new_cursor_pos);
            moveCursorLeft(ed);
        }
    } else {
        
    }

    
}