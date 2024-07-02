#ifndef _TERM_H
#define _TERM_H

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT                                                              \
    { NULL, 0 }

void disableRawMode();

void enableRawMode();

int getWindowSize(int *rows, int *cols);

void abAppend(struct abuf *ab, const char *s, int len);

void abFree(struct abuf *ab);

#endif
