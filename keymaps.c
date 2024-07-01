#include <stdlib.h>

#include "keymaps.h"
#include "main.h"

void addDefaultKeymaps(struct Keymaps *keymaps) {
    addKeymap(keymaps, CTRL_KEY('K'), &clearMessage);
}

struct Keymaps *createKeymapTable() {
    struct Keymaps *keymaps = malloc(sizeof(*keymaps));
    addDefaultKeymaps(keymaps);
    return keymaps;
}

void addKeymap(struct Keymaps *keymaps, char lhs, void *rhs) {
    keymaps->t[(int)lhs] = rhs;
}

void *find_keymap(struct Keymaps *keymaps, char lhs) {
    if (keymaps->t[(int)lhs] != NULL) {
        return keymaps->t[(int)lhs];
    }
    return NULL;
}
