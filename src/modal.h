#pragma once
#include "buffer.h"
#include "context.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "cursor.h"

typedef enum {
    MODAL_TYPE_OPTION,
    MODAL_TYPE_TEXT
} ModalType;

typedef struct {
    ModalType type;
    string prompt;
    union {
        bool selection;
        GapBuffer *buf;
    };
    Cursor cursor;
    bool submitted;

    // render info
    Rect frame;
    Vector2 title_pos;
    Vector2 text_pos;
    Rect input_box;
} Modal;

Modal *modalTextInit(const char *prompt, Vector2 cursor_pos);
Modal *modalOptionInit(const char *prompt);

void modalUpdate(Modal *modal, AppContext *ctx, f64 delta_time);
void modalDestroy(Modal *modal);

void modalSubmit(Modal *modal);
void modalCycleFocus(Modal *modal);

// // General modal movement
// void modalMoveCursorLeft(Modal *modal);
void modalMoveCursorRight(Modal *modal);
// void modalMoveCursorBeginning(Modal *modal);
// void modalMoveCursorEnd(Modal *modal);
//
// // Text modal specific
void modalInsert(Modal *modal, char *bytes);
void modalDeleteLeft(Modal *modal);
// void textmodalDeleteRight(Modal *modal);


