#ifndef _KEYMAPS_H
#define _KEYMAPS_H

#include "main.h"

struct Keymaps {
    void (*n[127])(void);
    void (*i[127])(void);
    void (*t[127])(void);
};

struct Keymaps *createKeymapTable();

void addKeymap(struct Keymaps *keymaps, enum Mode mode, char lhs, void *rhs);

void *find_keymap(struct Keymaps *keymaps, enum Mode mode, char lhs);

void noop();

#endif
