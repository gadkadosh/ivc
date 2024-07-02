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

void editorSwitchInsertMode() { E.mode = INSERT; }

void editorSwitchNormalMode() { E.mode = NORMAL; }
