#include "modal.h"
#include "buffer.h"
#include "cursor.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "stdlib.h"
#include <assert.h>
#include "grapheme.h"

Modal *modalTextInit(const char *prompt, Vector2 cursor_pos) {
    Modal *modal = (Modal *)malloc(sizeof(Modal));

    modal->type = MODAL_TYPE_TEXT;
    modal->prompt = stringNew(prompt);
    modal->buf = gapBufferNew(8);
    modal->submitted = false;
    modal->cursor = cursorNew(cursor_pos);
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

void modalDestroy(Modal *modal) {
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

void modalUpdate(Modal *modal, AppContext *ctx, f64 delta_time) {
    
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

void textmodalInsert(Modal *modal, char *bytes) {
    assert(modal->type == MODAL_TYPE_TEXT);
    size_t grapheme_size, offset = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);

        insertUnicodeCharIntoBuf (modal->buf, modal->cursor.buffer_idx, grapheme, grapheme_size);
    }
}
