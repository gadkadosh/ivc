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
    if (E.mode == INSERT) {
        if (cx < 0) {
            editorMoveCursor(ARROW_UP);
            row = &E.rows[E.cy];
            row->chars[row->size] = '\0';
            editorMoveCursorEnd();
            return;
        }
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

void editorAppendNewLine() {
    if (E.mode == INSERT) {
        if (E.cx == 0) {
            editorInsertRow(E.cy, "", 0);
        } else {
            struct erow *row = &E.rows[E.cy];
            editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
            row = &E.rows[E.cy];
            row->size = E.cx;
            row->chars[E.cx] = '\0';
            editorUpdateRow(row);
        }
    }
    E.cy++;
    E.cx = 0;
}

void editorSwitchNormalMode() { E.mode = NORMAL; }
