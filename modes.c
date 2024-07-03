#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "modes.h"

void editorRowInsertChar(struct erow *row, char c, int cx) {
    if (cx < 0 || cx > row->size)
        cx = row->size;
    row->size++;
    row->chars = realloc(row->chars, row->size + 1);
    memmove(&row->chars[cx + 1], &row->chars[cx], row->size - cx + 1);
    row->chars[cx] = c;

    editorUpdateRow(row);
}

void editorRowDeleteChar(struct erow *row, int cx) {
    if (cx < 0)
        return;
    if (E.mode == INSERT) {
        memmove(&row->chars[cx], &row->chars[cx + 1], row->size - cx);
        row->size--;
        editorUpdateRow(row);
    }
}

void editorSwitchInsertMode() { E.mode = INSERT; }

void editorSwitchInsertModeAppend() {
    editorMoveCursor(ARROW_RIGHT);
    editorSwitchInsertMode();
}

void editorSwitchInsertModeAppendEnd() {
    editorMoveCursorEnd();
    editorSwitchInsertMode();
}

void editorSwitchNormalMode() { E.mode = NORMAL; }
