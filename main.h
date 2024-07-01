#ifndef _MAIN_H
#define _MAIN_H

#include <termios.h>
#include <time.h>

#define VERSION "0.0.1"

#define TAB_STOP 8

#define CTRL_KEY(k) ((k) & 0x1f)

struct erow {
    int size;
    int rsize;
    char *chars;
    char *render;
};

struct EditorConfig {
    int cx, cy;
    int rx;
    int screenrows, screencols;
    struct termios orig_termios;
    int numrows;
    int rowoffset;
    int coloffset;
    struct erow *rows;
    char *filename;
    char message[80];
    time_t message_time;
    struct KeymapTable *keymaps;
};

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

void initEditor();

void die(const char *s);

int editorReadKey();

void clearMessage();

#endif
