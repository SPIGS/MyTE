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

void modalMoveCursorRight(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modal->cursor.moved_last_frame = true;
        if (modal->cursor.buffer_idx == getBufLength(modal->buf)) {
            modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
            return;
        }
        modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
        modal->cursor.buffer_idx = getNextGraphemeCursor(modal->buf, modal->cursor.buffer_idx);
        modal->cursor.pos_anim_time = 0.0f;
    } else {
        modalCycleFocus(modal);
    }
}

void modalMoveCursorLeft(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modal->cursor.moved_last_frame = true;
        if (modal->cursor.buffer_idx == 0) {
            modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
            return;
        }

        modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
        modal->cursor.buffer_idx = getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx);
        modal->cursor.pos_anim_time = 0.0f;
    } else {
        modalCycleFocus(modal);
    }
}

void modalMoveCursorBeginning(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modal->cursor.moved_last_frame = true;
        if (modal->cursor.buffer_idx == 0) {
            modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
            return;
        }

        modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
        modal->cursor.buffer_idx = getBeginningOfPrevLineCursor(modal->buf, modal->cursor.buffer_idx);
        modal->cursor.pos_anim_time = 0.0f;
    } else {
        modalCycleFocus(modal);
    }
}

void modalMoveCursorEnd(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modal->cursor.moved_last_frame = true;
        modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
        modal->cursor.buffer_idx = getBeginningOfNextLineCursor(modal->buf, modal->cursor.buffer_idx);
        modal->cursor.pos_anim_time = 0.0f;
    } else {
        modalCycleFocus(modal);
    }
}

void modalInsert(Modal *modal, char *bytes) {
    assert(modal->type == MODAL_TYPE_TEXT);
    modal->cursor.moved_last_frame = true;
    size_t grapheme_size, offset = 0;
    for (offset = 0; bytes[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(bytes + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(bytes+offset, grapheme_size);

        insertUnicodeCharIntoBuf (modal->buf, modal->cursor.buffer_idx, grapheme, grapheme_size);
        modalMoveCursorRight(modal);
    }
    modal->cursor.pos_anim_time = 0.0f;
}

void modalDeleteLeft(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modal->cursor.moved_last_frame = true;
        if (modal->cursor.buffer_idx != 0) {
            removeGraphemeBeforeGap(modal->buf, modal->cursor.buffer_idx);
            modal->cursor.prev_buffer_idx = modal->cursor.buffer_idx;
            modal->cursor.buffer_idx = getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx);
        }
        modal->cursor.pos_anim_time = 0.0f;
    }
}

void modalDeleteRight(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        removeGraphemeAfterGap(modal->buf, modal->cursor.buffer_idx);
    }
}

void modalMoveBegOfPrevWord(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        size_t prev_pos = modal->cursor.buffer_idx;
        modalMoveCursorLeft(modal);
        UnicodeChar c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));

        // skip spaces
        if (isspaceUTF8(c)) {
            while (isspaceUTF8(c) && c != '\n' && modal->cursor.buffer_idx != 0) {
                modalMoveCursorLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            } 
        }

        if (ispunctUTF8(c) && c != '_') {
            while (ispunctUTF8(c) && c != '_' && modal->cursor.buffer_idx != 0) {
                modalMoveCursorLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            }
        } else if (isalnumUTF8(c) || c == '_'){
            while ((isalnumUTF8(c) || c == '_') && modal->cursor.buffer_idx != 0) {
                modalMoveCursorLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            }
        }
        modal->cursor.prev_buffer_idx = prev_pos;
    }
}

void modalMoveCursorEndOfNextWord(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        size_t prev_pos = modal->cursor.buffer_idx;
        modalMoveCursorRight(modal);
        UnicodeChar c = getBufChar(modal->buf, modal->cursor.buffer_idx);
        size_t buf_len = getBufLength(modal->buf);
        // skip spaces
        if (isspaceUTF8(c)) {
            while(isspaceUTF8(c) && c != '\n' && modal->cursor.buffer_idx != buf_len) {
                modalMoveCursorRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            }
        }

        if (ispunctUTF8(c) && c != '_') {
            while (ispunctUTF8(c) && c != '_' && modal->cursor.buffer_idx != buf_len) {
                modalMoveCursorRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            }
        } else if (isalnumUTF8(c) || c == '_') {
            while ((isalnumUTF8(c) || c == '_') && modal->cursor.buffer_idx != buf_len) {
                modalMoveCursorRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            }
        }
        modal->cursor.prev_buffer_idx = prev_pos;
    }
}

void modalDeleteWordLeft(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        modalDeleteLeft(modal);
        char c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));

        // skip spaces
        if (isspaceUTF8(c)) {
            while (isspaceUTF8(c) && c != '\n' && modal->cursor.buffer_idx != 0) {
                modalDeleteLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            } 
        }

        if (ispunctUTF8(c) && c != '_') {
            while (ispunctUTF8(c) && c != '_' && modal->cursor.buffer_idx != 0) {
                modalDeleteLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            }
        } else if (isalnumUTF8(c) || c == '_'){
            while ((isalnumUTF8(c) || c == '_') && modal->cursor.buffer_idx != 0) {
                modalDeleteLeft(modal);
                c = getBufChar(modal->buf, getPrevGraphemeCursor(modal->buf, modal->cursor.buffer_idx));
            }
        }
    }
}

void modalDeleteWordRight(Modal *modal) {
    if (modal->type == MODAL_TYPE_TEXT) {
        char c = getBufChar(modal->buf, modal->cursor.buffer_idx);

        // We don't want to skip here - if there are spaces we want to delete those
        // and let the user choose to delete more.
        if (c == '\n') {
            modalDeleteRight(modal);
        } else if (isspaceUTF8(c) && c != '\n') {
            while (isspaceUTF8(c) && c != '\n') {
                modalDeleteRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            } 
        } else if (ispunctUTF8(c) && c != '_') {
            while (ispunctUTF8(c) && c != '_') {
                modalDeleteRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            }
        } else if (isalnumUTF8(c) || c == '_'){
            while (isalnumUTF8(c) || c == '_') {
                modalDeleteRight(modal);
                c = getBufChar(modal->buf, modal->cursor.buffer_idx);
            }
        }
    }
}
