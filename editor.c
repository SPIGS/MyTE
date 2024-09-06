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


// TODO CHange this to be like renderer - pass this by reference
Editor *editorInit(f32 x, f32 y, f32 width, f32 height,f32 line_height) {
    Editor *ed = (Editor*)malloc(sizeof(Editor));
    ed->buf = gapBufferInit(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorInit(vec2_init(x, height));
    ed->goal_column = -1;
    ed->frame = rect_init(x, y, width, height);
    ed->text_pos = vec2_init(x,y + height);
    ed->scroll_pos = vec2_init(0,0);
    ed->line_count = 1;
    ed->line_height = line_height;
    return ed;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
}

// TODO: keep track of cursor row and column as it moves.

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

// TODO: make a favored column like how emacs has it
void moveCursorUp(Editor *ed) {
    if (ed->goal_column == -1) {
        setGoalColumn(ed);
    }
    ed->cursor.prev_buffer_pos = ed->cursor.buffer_pos;
    size_t beginning_of_prev_line = getBeginningOfPrevLineCursor(ed->buf, ed->cursor.buffer_pos);
    size_t prev_line_length = getBufLineLength(ed->buf, beginning_of_prev_line);
    if (getBeginningOfLineCursor(ed->buf, ed->cursor.buffer_pos) == 0) {
        prev_line_length = 0;
    }
    ed->cursor.buffer_pos = beginning_of_prev_line + MIN(prev_line_length, (size_t)ed->goal_column);
    ed->cursor.disp_column = MIN(prev_line_length, (size_t)ed->goal_column);
    if (ed->cursor.disp_column < 1) {
        ed->cursor.disp_column = 1;
    }

    ed->cursor.anim_time = 0.0f;

    if (ed->cursor.disp_row > 1) {
        ed->cursor.disp_row--;
    }
}

// TODO: make a favored column like how emacs has it
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

    if (ed->cursor.buffer_pos != ed->cursor.prev_buffer_pos) {
        ed->cursor.disp_row++;
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
}

// TODO handle deletion of lines more elloquently
// currently tracking the line count of the document is all over the place
// maybe create a function to keep track?
// Add that function here - currently deletin from the right doesn't decrement
// the line count.
void deleteCharacterRight(Editor *ed) {
    if (removeCharAfterGap (ed->buf, ed->cursor.buffer_pos) == '\n') {
        ed->line_count = (ed->line_count - 1 < 1) ? 1 : ed->line_count - 1;
        printf("removed newline\n");
    }
}

char *getContents(Editor *ed) {
    return getBufString(ed->buf);
}

void setGoalColumn(Editor *ed) {
    ed->goal_column = getBufColumn(ed->buf, ed->cursor.buffer_pos);
    ed->cursor.disp_column = ed->goal_column+1;
}

void updateScroll(Editor *ed) {

    f32 SCROLL_UP_LINE_FACTOR = 3;
    // If  the cursor is moving down
    if (ed->cursor.target_screen_pos.y <= ed->frame.y) {
        ed->cursor.target_screen_pos.y = ed->frame.y;
        ed->scroll_pos.y += ed->line_height;

    // If the cursor is moving up

    } else if (ed->cursor.target_screen_pos.y >= (ed->frame.y + ed->frame.h - (SCROLL_UP_LINE_FACTOR * ed->line_height)) && ed->scroll_pos.y > 0.0) {
        ed->cursor.target_screen_pos.y = (ed->frame.y + ed->frame.h) - ed->line_height;
        ed->scroll_pos.y -= ed->line_height;
    }
}

void updateFrame(Editor *ed, f32 screen_width, f32 screen_height) {
    f32 STATUS_LINE_HEIGHT = ed->line_height + 5;

    ed->frame = rect_init(0,0 + STATUS_LINE_HEIGHT, screen_width, screen_height - STATUS_LINE_HEIGHT);
    ed->text_pos = vec2_init(0, 0 + screen_height);
}