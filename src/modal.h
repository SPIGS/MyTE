#pragma once
#include "buffer.h"
#include "putils/pstring.h"

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
    bool submitted;
} Modal;

Modal *modalTextInit(const char *prompt);
Modal *modalOptionInit(const char *prompt);

void ModalDestroy(Modal *modal);

void modalSubmit(Modal *modal);
void modalCycleFocus(Modal *modal);

// // General modal movement
// void modalMoveCursorLeft(Modal *modal);
// void modalMoveCursorRight(Modal *modal);
// void modalMoveCursorBeginning(Modal *modal);
// void modalMoveCursorEnd(Modal *modal);
//
// // Text modal specific
// void textmodalInsert(Modal *modal);
// void textmodalDeleteLeft(Modal *modal);
// void textmodalDeleteRight(Modal *modal);


