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

void editorInit(Editor *ed, rect frame, f32 line_height) {
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorInit(vec2_init(frame.x, frame.h));
    ed->goal_column = -1;
    ed->frame = rect_init(frame.x, frame.y, frame.w, frame.h);
    ed->text_pos = vec2_init(frame.x, frame.y + frame.h);
    ed->scroll_pos = vec2_init(0,0);
    ed->line_count = 1;
    ed->line_height = line_height;
    ed->file_path = NULL;
    lexerInit(&ed->lexer);
    ed->dirty = true;
    ed->tab_stop = 4;
    ed->cursor_speed = 3.5;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
    lexerDestroy(&ed->lexer);
}

void editorLoadConfig(Editor *ed, Config *config) {
    ed->tab_stop = config->tab_stop;
    ed->cursor_speed = config->cursor_speed;
    ed->dirty = true;
}

// Cursor Movements
void moveCursorLeft(Editor *ed) {
     ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
     ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
     ed->cursor.anim_time = 0.0f;
     setGoalColumn(ed);

     if (getBufChar(ed->buf, ed->cursor.buffer_pos) == '\n' && ed->cursor.disp_row > 1) {
        ed->cursor.disp_row--;
     }
}

void moveCursorRight(Editor *ed) {
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    ed->cursor.buffer_pos = getNextCharCursor(ed->buf, ed->cursor.buffer_pos);
    ed->cursor.anim_time = 0.0f;
    setGoalColumn(ed);

    if (getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos)) == '\n' && ed->cursor.buffer_pos != ed->cursor.prev_buffer_pos) {
        ed->cursor.disp_row++;
    }
}

void moveCursorUp(Editor *ed) {
    
    // Set the goal column if we haven't already
    if (ed->goal_column == -1) {
        setGoalColumn(ed);
    }

    // Find the beginning of the prev line and the lenght of the prev line
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    size_t beginning_of_prev_line = getBeginningOfPrevLineCursor(ed->buf, ed->cursor.buffer_pos);
    size_t prev_line_length = getBufLineLength(ed->buf, beginning_of_prev_line);

    // if the position of the beginning of current line is 0, that means we are on the first line
    // we should act as if the length of the previous line is 0.
    if (getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_pos) == 0) {
        prev_line_length = 0;
    }

    // If the length of the prev line is 0, we should just move to the end of the previous line
    if (prev_line_length == 0) {
        ed->cursor.buffer_pos = getEndOfPrevLineCursor(ed->buf, ed->cursor.buffer_pos);
        ed->cursor.disp_column = MIN(prev_line_length, (size_t)ed->goal_column);
    } else {
        ed->cursor.buffer_pos = beginning_of_prev_line + MIN(prev_line_length, (size_t)ed->goal_column);
        ed->cursor.disp_column = MIN(prev_line_length, (size_t)ed->goal_column);
    }

    if (ed->cursor.disp_column < 1) {
        ed->cursor.disp_column = 1;
    }

    ed->cursor.anim_time = 0.0f;

    if (ed->cursor.disp_row > 1) {
        ed->cursor.disp_row--;
    } else if (ed->cursor.disp_row == 1) {
        setGoalColumn(ed);
    }
}

void moveCursorDown(Editor *ed) {
    if (ed->goal_column == -1) {
        setGoalColumn(ed);
    }
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    size_t beginning_of_next_line = getBeginningOfNextLineCursor(ed->buf, ed->cursor.buffer_pos);
    size_t next_line_length = getEndOfLineCursor(ed->buf, beginning_of_next_line) - beginning_of_next_line;
    ed->cursor.buffer_pos = beginning_of_next_line + MIN(next_line_length, (size_t)ed->goal_column);
    ed->cursor.disp_column = MIN(next_line_length, (size_t)ed->goal_column);
    if (ed->cursor.disp_column < 1) {
        ed->cursor.disp_column = 1;
    }
    ed->cursor.anim_time = 0.0f;

    if (ed->cursor.disp_row < ed->line_count) {
        ed->cursor.disp_row++;
    } else if (ed->cursor.disp_row == ed->line_count) {
        setGoalColumn(ed);
    }
}

void insertCharacter(Editor *ed, char character, bool move_cursor_forward) {
    if (character == '\n') {
        ed->line_count ++;
        ed->cursor.disp_row++;
    }

    insertCharIntoBuf(ed->buf, ed->cursor.buffer_pos, character);
    if (move_cursor_forward) {
        ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
        ed->cursor.buffer_pos = getNextCharCursor(ed->buf, ed->cursor.buffer_pos);
        setGoalColumn(ed);
        ed->cursor.anim_time = 0.0f;
    }
    ed->dirty = true;
}

void deleteCharacterLeft(Editor *ed) {
    if (getBufChar(ed->buf, getPrevCharCursor(ed->buf, ed->cursor.buffer_pos)) == '\n' && ed->line_count > 1) {
        ed->line_count--;
        ed->cursor.disp_row--;
    }

    removeCharBeforeGap (ed->buf, ed->cursor.buffer_pos);
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    ed->cursor.buffer_pos = getPrevCharCursor(ed->buf, ed->cursor.buffer_pos);
    setGoalColumn(ed);
    ed->cursor.anim_time = 0.0f;
    ed->dirty = true;
}

// TODO handle deletion of lines more elloquently
// currently tracking the line count of the document is all over the place
// maybe create a function to keep track?
void deleteCharacterRight(Editor *ed) {
    if (removeCharAfterGap (ed->buf, ed->cursor.buffer_pos) == '\n') {
        ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
    }
    ed->dirty = true;
}

char *getContents(Editor *ed) {
    return getBufString(ed->buf);
}

void setGoalColumn(Editor *ed) {
    ed->goal_column = getBufColumn(ed->buf, ed->cursor.buffer_pos);
    ed->cursor.disp_column = ed->goal_column+1;
}

void editorUpdate(Editor *ed, f32 screen_width, f32 screen_height, ColorTheme theme, f64 delta_time) {
    UNUSED(delta_time);

    // Update scroll
    f32 SCROLL_UP_LINE_FACTOR = 6;
    f32 SCROLL_DOWN_LINE_FACTOR = 2;
    // If  the cursor is moving down
    if (ed->cursor.target_screen_pos.y <= (ed->frame.y + (SCROLL_DOWN_LINE_FACTOR * ed->line_height)) && ed->scroll_pos.y < (ed->line_height * ed->line_count)) {
        ed->cursor.target_screen_pos.y = ed->frame.y;
        ed->scroll_pos.y += ed->line_height;

    // If the cursor is moving up
    } else if (ed->cursor.target_screen_pos.y >= (ed->frame.y + ed->frame.h - (SCROLL_UP_LINE_FACTOR * ed->line_height)) && ed->scroll_pos.y > 0.0) {
        ed->cursor.target_screen_pos.y = (ed->frame.y + ed->frame.h) - ed->line_height;
        ed->scroll_pos.y -= ed->line_height;
    }

    // Update frame
    f32 STATUS_LINE_HEIGHT = ed->line_height;
    ed->frame = rect_init(0,0 + STATUS_LINE_HEIGHT, screen_width, screen_height - STATUS_LINE_HEIGHT);
    ed->text_pos = vec2_init(0, 0 + screen_height);

    // Update the lexer
    if (ed->dirty && ed->lexer.file_type != FILE_TYPE_UNKNOWN) {
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
    if (!ed->file_path)
        LOG_ERROR("Writing a blank file to disk is not supported!", "");

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
}

void moveCursorWordBackward(Editor *ed) {
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
}