#ifndef _KEYMAPS_H
#define _KEYMAPS_H

struct Keymap {
    char lhs;
    void (*rhs)(void);
};

struct KeymapTable {
    int size;
    int capacity;
    struct Keymap *table;
};

struct KeymapTable *createKeymapTable();

void addKeymap(struct KeymapTable *keymaps, char rhs, void *lhs);

void *find_keymap(struct KeymapTable *keymaps, char rhs);

#endif
