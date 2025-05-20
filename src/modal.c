#include "modal.h"
#include "buffer.h"
#include "putils/pstring.h"
#include "stdlib.h"

Modal *modalTextInit(const char *prompt) {
    Modal *modal = (Modal *)malloc(sizeof(Modal));

    modal->type = MODAL_TYPE_TEXT;
    modal->prompt = stringNew(prompt);
    modal->buf = gapBufferNew(8);
    modal->submitted = false;
    return modal;
}

Modal *modalOptionInit(const char *prompt) {
    Modal *modal = (Modal *)malloc(sizeof(Modal));

    modal->type = MODAL_TYPE_OPTION;
    modal->prompt = stringNew(prompt);
    modal->selection = true;
    modal->submitted = false;

    return modal;
}

void ModalDestroy(Modal *modal) {
    // Free the prompt
    stringFree(modal->prompt);

    switch (modal->type) {
        case MODAL_TYPE_TEXT:
            gapBufferDestroy(modal->buf);
        break;
        default:
        break;
    }
    free(modal);
}

void modalSubmit(Modal *modal) {
    modal->submitted = true;
}

void modalCycleFocus(Modal *modal) {
    switch(modal->type) {
        case MODAL_TYPE_OPTION:
            modal->selection = !modal->selection;
        break;
        case MODAL_TYPE_TEXT:
        break;
    }
}
