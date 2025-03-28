#include <stdlib.h>
#include "editor.h"
#include "buffer.h"
#include "cursor.h"

Editor *editorNew(void) {
    Editor *ed = (Editor *)malloc(sizeof(Editor));
    ed->buf = gapBufferNew(INITIAL_BUFFER_SIZE);
    ed->cursor = cursorNew();
    return ed;
}

void editorDestroy(Editor *ed) {
    gapBufferDestroy(ed->buf);
    free(ed);
}

void editorMoveLeft(Editor *ed) {
    ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
}

void editorMoveRight(Editor *ed) {
    ed->cursor.buffer_idx = getNextGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
}

void editorInsert(Editor *ed, char *bytes) {
    ed->cursor.buffer_idx = insertIntoBuf(ed->buf, ed->cursor.buffer_idx, bytes);
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
}

void editorDeleteLeft(Editor *ed) {
    removeGraphemeBeforeGap(ed->buf, ed->cursor.buffer_idx);
    ed->cursor.buffer_idx = getPrevGraphemeCursor(ed->buf, ed->cursor.buffer_idx);
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
}

void editorDeleteRight(Editor *ed) {
    removeGraphemeAfterGap(ed->buf, ed->cursor.buffer_idx);
    outputBufferString(ed->buf, ed->cursor.buffer_idx);
}
