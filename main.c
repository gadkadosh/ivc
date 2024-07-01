#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "main.h"
#include "term.h"

#define CTRL_KEY(k) ((k) & 0x1f)

struct EditorConfig E;

void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

int editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, sizeof(char))) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] > '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        } else if (seq[0] == '0') {
            switch (seq[1]) {
            case 'H':
                return ARROW_UP;
            case 'F':
                return ARROW_DOWN;
            }
        }

        return '\x1b';
    }

    return c;
}

void editorMoveCursor(int c) {
    struct erow *cur = E.cy > E.numrows ? NULL : &E.rows[E.cy];

    switch (c) {
    case 'g':
        E.rowoffset = 0;
        E.cy = 0;
        break;
    case 'G':
        E.rowoffset = E.numrows - E.screenrows;
        E.cy = E.numrows;
        break;
    case 'h':
    case ARROW_LEFT:
        if (E.cx != 0)
            E.cx--;
        break;
    case 'j':
    case ARROW_DOWN:
        if (E.cy < E.numrows)
            E.cy++;
        break;
    case 'k':
    case ARROW_UP:
        if (E.cy != 0)
            E.cy--;
        break;
    case 'l':
    case ARROW_RIGHT:
        if (cur && cur->size > E.cx)
            E.cx++;
        break;
    }

    cur = E.cy > E.numrows ? NULL : &E.rows[E.cy];
    int rowlen = cur ? cur->size : 0;
    if (E.cx > rowlen)
        E.cx = rowlen;
}

void editorProcessKeypresses() {
    int c = editorReadKey();

    switch (c) {
    case 'q':
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;

    case '0':
    case HOME_KEY:
        E.cx = 0;
        break;
    case '$':
    case END_KEY:
        E.cx = E.rows[E.cy].size - 1;
        break;

    case PAGE_UP:
    case PAGE_DOWN: {
        int times = E.screenrows;
        while (times--)
            editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    } break;

    case 'g':
    case 'G':
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
    case ARROW_LEFT:
        editorMoveCursor(c);
        break;
    }
}

void drawStatusline(struct abuf *ab) {
    char status[32];
    int len = snprintf(status, sizeof status, "%d:%d", E.cy + 1, E.cx + 1);
    int padding = E.screencols;
    while (padding - len > 0) {
        abAppend(ab, " ", 1);
        padding--;
    }
    abAppend(ab, status, len);
}

void editorDrawRows(struct abuf *ab) {
    for (int i = 0; i < E.screenrows; i++) {
        int filerow = E.rowoffset + i;
        if (i == E.screenrows - 1) {
            drawStatusline(ab);
        } else if (filerow >= E.numrows) {
            if (E.numrows == 0 && i == E.screenrows / 3) {
                char welcome[80];
                int welcomelen =
                    sprintf(welcome, "C Editor -- version %s", VERSION);
                if (welcomelen > E.screencols)
                    welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) {
                    abAppend(ab, " ", 1);
                }
                abAppend(ab, welcome, welcomelen);
            } else {
                abAppend(ab, "~", 1);
            }
        } else {
            int len = E.rows[filerow].size - E.coloffset;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;
            abAppend(ab, &E.rows[filerow].chars[E.coloffset], len);
        }

        abAppend(ab, "\x1b[K", 3);
        if (i < E.screenrows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorScroll() {
    if (E.cy < E.rowoffset) {
        E.rowoffset = E.cy;
    }
    if (E.cy >= E.rowoffset + E.screenrows) {
        E.rowoffset = E.cy - E.screenrows + 1;
    }
    if (E.cx < E.coloffset) {
        E.coloffset = E.cx;
    }
    if (E.cx >= E.coloffset + E.screencols) {
        E.coloffset = E.cx - E.screencols + 1;
    }
}

void editorRefreshScreen() {
    editorScroll();

    struct abuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof buf, "\x1b[%d;%dH", E.cy - E.rowoffset + 1,
             E.cx - E.coloffset + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.rowoffset = 0;
    E.coloffset = 0;
    E.rows = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    };
}

void editorAppendRow(char *line, size_t len) {
    E.rows = realloc(E.rows, sizeof(struct erow) * (E.numrows + 1));
    E.rows[E.numrows].size = len;
    E.rows[E.numrows].chars = malloc(len + 1);
    memcpy(E.rows[E.numrows].chars, line, len);
    E.rows[E.numrows].chars[len] = '\0';
    E.numrows++;
}

void editorOpen(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp)
        die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 &&
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editorAppendRow(line, linelen);
    }

    free(line);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    while (1) {
        editorRefreshScreen();
        editorProcessKeypresses();
    }

    return 0;
}
