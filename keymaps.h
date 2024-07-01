#ifndef _KEYMAPS_H
#define _KEYMAPS_H

struct Keymaps {
    void (*t[127])(void);
};

struct Keymaps *createKeymapTable();

void addKeymap(struct Keymaps *keymaps, char rhs, void *lhs);

void *find_keymap(struct Keymaps *keymaps, char rhs);

#endif
