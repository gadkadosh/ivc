#include <stdlib.h>

#include "keymaps.h"
#include "main.h"
#include "modes.h"

void addDefaultKeymaps(struct Keymaps *keymaps) {
    addKeymap(keymaps, NORMAL, CTRL_KEY('k'), &clearMessage);
    addKeymap(keymaps, INSERT, CTRL_KEY('c'), &editorSwitchNormalMode);
    addKeymap(keymaps, NORMAL, 'i', &editorSwitchInsertMode);
    addKeymap(keymaps, NORMAL, 'a', &editorSwitchInsertModeAppend);
    addKeymap(keymaps, NORMAL, 'A', &editorSwitchInsertModeAppendEnd);
}

struct Keymaps *createKeymapTable() {
    struct Keymaps *keymaps = malloc(sizeof(*keymaps));
    addDefaultKeymaps(keymaps);
    return keymaps;
}

void addKeymap(struct Keymaps *keymaps, enum Mode mode, char lhs, void *rhs) {
    switch (mode) {
    case NORMAL:
        keymaps->n[(int)lhs] = rhs;
        break;
    case INSERT:
        keymaps->i[(int)lhs] = rhs;
        break;
    }
}

void noop() {}

void *find_keymap(struct Keymaps *keymaps, enum Mode mode, char lhs) {
    if (keymaps->t[(int)lhs] != NULL) {
        return keymaps->t[(int)lhs];
    }
    switch (mode) {
    case NORMAL:
        return keymaps->n[(int)lhs];
    case INSERT:
        return keymaps->i[(int)lhs];
    default:
        return NULL;
    }
}
