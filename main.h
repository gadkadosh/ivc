#ifndef _MAIN_H
#define _MAIN_H

#include <termios.h>
#include <time.h>

#define VERSION "0.0.1"

#define TAB_STOP 8

#define CTRL_KEY(k) ((k) & 0x1f)

extern struct EditorConfig E;

enum Mode { NORMAL, INSERT };

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
    int dirty;
    char message[80];
    time_t message_time;
    struct Keymaps *keymaps;
    enum Mode mode;
};

enum editorKey {
    BACKSPACE = 127,
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

void writeFile();

int editorReadKey();

void editorMoveCursor(int c);

void editorMoveCursorEnd();

void editorUpdateRow(struct erow *row);

void clearMessage();

void editorAppendRow(char *line, size_t len);

void editorSetStatusMessage(const char *fmt, ...);

#endif
