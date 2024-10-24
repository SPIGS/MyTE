#include "dialog.h"

static void dialogCalculateDims(SaveDialog *sd, AppContext *ctx) {
    f32 width = MIN(500.0, ctx->screen_width * 0.5);
    f32 height = sd->line_height * 3;
    f32 x = (ctx->screen_width / 2) - (width / 2);
	f32 y = (ctx->screen_height / 2) - (height / 2);

    rect frame = rect_init(x, y, width, height);
    vec2 title_pos = vec2_init(x + ctx->glyph_adv, y + ctx->line_height * 2);
    rect input_box = rect_init(x + ctx->glyph_adv, y + ctx->line_height * 0.75, width - ctx->glyph_adv * 2, ctx->line_height);
    vec2 text_pos = vec2_init(input_box.x, input_box.y + ctx->descender);

    sd->frame = frame;
    sd->title_pos = title_pos;
    sd->text_pos = text_pos;
    sd->input_box = input_box;
}

SaveDialog dialogInit(vec2 cursor_pos, f32 line_height, f32 glyph_adv) {
    SaveDialog sd = (SaveDialog) {
        .buf = gapBufferInit(INITIAL_BUFFER_SIZE),
        .cursor = cursorInit(cursor_pos, 0.5),
        .line_height = line_height,
        .glyph_adv = glyph_adv
    };
    return sd;
}
void dialogDestroy(SaveDialog *sd) {
    gapBufferDestroy(sd->buf);
}

void dialogUpdate(SaveDialog *sd, AppContext *ctx, f64 delta_time) {
    dialogCalculateDims(sd, ctx);

    vec2 adj_cursor_pos = vec2_init(sd->input_box.x + ctx->glyph_adv * (sd->cursor.disp_column - 1), sd->input_box.y);
     cursorUpdate(&sd->cursor, adj_cursor_pos, delta_time);
}

void dialogMoveCursorLeft(SaveDialog *sd) {
    sd->cursor.moved_last_frame = true;
    if (sd->cursor.buffer_pos == 0) {
        sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
        return;
    }

    sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
    sd->cursor.buffer_pos = getPrevCharCursor(sd->buf, sd->cursor.buffer_pos);
    sd->cursor.disp_column = getBufColumn(sd->buf, sd->cursor.buffer_pos) + 1;
    sd->cursor.pos_anim_time = 0.0f;
}

void dialogMoveCursorRight(SaveDialog *sd) {
    sd->cursor.moved_last_frame = true;
    if (sd->cursor.buffer_pos == getBufLength(sd->buf)) {
        sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
        return;
    }

    sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
    sd->cursor.buffer_pos = getNextCharCursor(sd->buf, sd->cursor.buffer_pos);
    sd->cursor.disp_column = getBufColumn(sd->buf, sd->cursor.buffer_pos) + 1;
    sd->cursor.pos_anim_time = 0.0f;
}

void dialogMoveCursorBeginning(SaveDialog *sd) {
    sd->cursor.moved_last_frame = true;
    sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
    sd->cursor.buffer_pos = 0;
    sd->cursor.disp_column = getBufColumn(sd->buf, sd->cursor.buffer_pos) + 1;
    sd->cursor.pos_anim_time = 0.0f;
}

void dialogMoveCursorEnd(SaveDialog *sd) {
    sd->cursor.moved_last_frame = true;
    sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
    sd->cursor.buffer_pos = getBufLength(sd->buf);
    sd->cursor.disp_column = getBufColumn(sd->buf, sd->cursor.buffer_pos) + 1;
    sd->cursor.pos_anim_time = 0.0f;
}

void dialogInsertCharacter(SaveDialog *sd, char character) {
    // if (sd->cursor.selection_size != 0) {
    //     editorDeleteSelection(ed);
    // } else {
    //     editorUnselectSelection(ed);
    // }
    if (character != '\n') {
        insertCharIntoBuf(sd->buf, sd->cursor.buffer_pos, character);
    }
    dialogMoveCursorRight(sd);
}

void dialogDeleteCharLeft(SaveDialog *sd) {
    //editorUnselectSelection(ed);
    sd->cursor.moved_last_frame = true;
    if (sd->cursor.buffer_pos != 0) {
        removeCharBeforeGap (sd->buf, sd->cursor.buffer_pos);
        sd->cursor.prev_buffer_pos = sd->cursor.buffer_pos;
        sd->cursor.buffer_pos = getPrevCharCursor(sd->buf, sd->cursor.buffer_pos);
        sd->cursor.disp_column = getBufColumn(sd->buf, sd->cursor.buffer_pos) + 1;
        sd->cursor.pos_anim_time = 0.0f;
    }
}

