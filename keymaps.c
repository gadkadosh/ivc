#include <stdlib.h>

#include "keymaps.h"
#include "main.h"

int hash(char key) {
    int hash = 135792;
    hash = hash ^ key;
    return hash;
}

struct KeymapTable *createKeymapTable() {
    struct KeymapTable *keymaps = malloc(sizeof(*keymaps));
    keymaps->size = 0;
    keymaps->capacity = 10;
    keymaps->table = malloc(keymaps->capacity * sizeof(struct Keymap));

    addKeymap(keymaps, CTRL_KEY('K'), &clearMessage);

    return keymaps;
}

void addKeymap(struct KeymapTable *keymaps, char lhs, void *rhs) {
    if (keymaps->size >= keymaps->capacity) {
        keymaps->capacity += 5;
        keymaps->table = realloc(keymaps->table, keymaps->capacity);
    }

    int h = hash(lhs);

    keymaps->table[h % keymaps->capacity].lhs = lhs;
    keymaps->table[h % keymaps->capacity].rhs = rhs;
}

void *find_keymap(struct KeymapTable *keymaps, char lhs) {
    for (int i = 0; i < keymaps->capacity; i++) {
        if (keymaps->table[i].lhs == lhs) {
            return keymaps->table[i].rhs;
        }
    }
    return NULL;
}
