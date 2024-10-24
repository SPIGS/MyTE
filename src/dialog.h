#ifndef SAVE_H
#define SAVE_H
#include "util.h"
#include "gapbuffer.h"
#include "cursor.h"
#include "context.h"

typedef struct {
    GapBuffer *buf;
    Cursor cursor;

    // render info
    f32 line_height;
    f32 glyph_adv;
    rect frame;
    vec2 title_pos;
    vec2 text_pos;
    rect input_box;
} SaveDialog;

SaveDialog dialogInit(vec2 cursor_pos, f32 line_height, f32 glyph_adv);
void dialogDestroy(SaveDialog *sd);
void dialogUpdate(SaveDialog *sd, AppContext *ctx, f64 delta_time);

void dialogMoveCursorLeft(SaveDialog *sd);
void dialogMoveCursorRight(SaveDialog *sd);
void dialogMoveCursorBeginning(SaveDialog *sd);
void dialogMoveCursorEnd(SaveDialog *sd);

void dialogInsertCharacter(SaveDialog *sd, char character);
void dialogDeleteCharLeft(SaveDialog *sd);

#endif