#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "keymaps.h"
#include "main.h"
#include "modes.h"
#include "term.h"

struct EditorConfig E;

void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.message, sizeof(E.message), fmt, ap);
    va_end(ap);
    time(&E.message_time);
}

void clearMessage() { editorSetStatusMessage(""); }

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

void editorMoveCursorEnd() {
    if (E.cy < E.numrows)
        E.cx = E.rows[E.cy].size;
}

void editorDelChar() {
    if (E.cx >= 0) {
        editorRowDeleteChar(&E.rows[E.cy], E.cx - 1);
        E.cx--;
    }
}

void editorInsertChar(char c) {
    if (E.cy == E.numrows) {
        editorInsertRow(E.numrows, "", 0);
    }
    editorRowInsertChar(&E.rows[E.cy], c, E.cx);
    E.dirty++;
    E.cx++;
}

void editorProcessKeypresses() {
    int c = editorReadKey();

    switch (E.mode) {
    case NORMAL: {
        void (*rhs)(void) = find_keymap(E.keymaps, NORMAL, c);
        if (rhs != NULL) {
            rhs();
            return;
        }
        break;
    }
    case INSERT: {
        void (*rhs)(void) = find_keymap(E.keymaps, INSERT, c);
        if (rhs != NULL) {
            rhs();
            return;
        }
        break;
    }
    }

    static int quit = 0;

    switch (c) {
    case CTRL_KEY('q'):
        if (E.mode == NORMAL) {
            if (E.dirty > 0 && quit == 0) {
                editorSetStatusMessage(
                    "Quit without saving? (C-q again to confirm)");
                quit++;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
        }
        break;
    case '\r':
        break;
    case '0':
    case HOME_KEY:
        E.cx = 0;
        break;
    case '$':
    case END_KEY:
        if (E.cy < E.numrows)
            E.cx = E.rows[E.cy].size;
        break;

    case PAGE_UP:
    case PAGE_DOWN: {
        if (c == PAGE_UP) {
            E.cy = E.rowoffset;
        } else if (c == PAGE_DOWN) {
            E.cy = E.rowoffset + E.screenrows - 1;
            if (E.cy > E.numrows)
                E.cy = E.numrows;
        }
        int times = E.screenrows;
        while (times--)
            editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    } break;

    case BACKSPACE:
    case DEL_KEY:
    case CTRL_KEY('h'):
        if (c == DEL_KEY)
            editorMoveCursor(ARROW_RIGHT);
        editorDelChar();
        break;

    case CTRL_KEY('l'):
    case '\x1b':
        break;

    case CTRL_KEY('s'):
        writeFile();
        break;

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
    default:
        if (E.mode == INSERT) {
            editorInsertChar(c);
        }
        break;
    }

    quit = 0;
}

void editorDrawMessageLine(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3);
    if (E.mode == INSERT)
        abAppend(ab, "-- INSERT --", 12);
    else if (time(NULL) - E.message_time < 5)
        abAppend(ab, E.message, strlen(E.message));
}

void editorDrawStatusline(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4);

    char lstatus[80], rstatus[80];
    int filenamelen = snprintf(lstatus, sizeof lstatus, "%s (%d lines) %s",
                               E.filename ? E.filename : "[No Name]", E.numrows,
                               E.dirty ? "[+]" : "");
    abAppend(ab, lstatus, filenamelen + 1);
    int len = snprintf(rstatus, sizeof rstatus, "%d:%d", E.cy + 1, E.cx + 1);
    int padding = E.screencols - filenamelen;
    while (padding - len > 0) {
        abAppend(ab, " ", 1);
        padding--;
    }
    abAppend(ab, rstatus, len);
    abAppend(ab, "\x1b[m", 3);
}

void editorDrawRows(struct abuf *ab) {
    for (int i = 0; i < E.screenrows; i++) {
        int filerow = E.rowoffset + i;
        if (filerow >= E.numrows) {
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
            int len = E.rows[filerow].rsize - E.coloffset;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;
            abAppend(ab, &E.rows[filerow].render[E.coloffset], len);
        }

        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

int editorRowCxToRx(struct erow *row, int cx) {
    int rx = 0;
    for (int i = 0; i < cx; i++) {
        if (row->chars[i] == '\t')
            rx += TAB_STOP - rx % TAB_STOP;
        else
            rx++;
    }

    return rx;
}

void editorScroll() {
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.rows[E.cy], E.cx);
    }

    if (E.cy < E.rowoffset) {
        E.rowoffset = E.cy;
    }
    if (E.cy >= E.rowoffset + E.screenrows) {
        E.rowoffset = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloffset) {
        E.coloffset = E.rx;
    }
    if (E.rx >= E.coloffset + E.screencols) {
        E.coloffset = E.rx - E.screencols + 1;
    }
}

void editorRefreshScreen() {
    editorScroll();

    struct abuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    editorDrawStatusline(&ab);
    abAppend(&ab, "\r\n", 2);
    editorDrawMessageLine(&ab);

    char buf[32];
    snprintf(buf, sizeof buf, "\x1b[%d;%dH", E.cy - E.rowoffset + 1,
             E.rx - E.coloffset + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.numrows = 0;
    E.rowoffset = 0;
    E.coloffset = 0;
    E.rows = NULL;
    E.filename = NULL;
    E.dirty = 0;
    E.message[0] = '\0';
    E.message_time = 0;
    E.mode = NORMAL;

    editorSetStatusMessage("Welcome");

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    }
    E.screenrows -= 2;

    E.keymaps = createKeymapTable();
}

void editorUpdateRow(struct erow *row) {
    int tabs = 0;
    for (int i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t')
            tabs++;
    }

    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int j = 0;
    for (int i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            row->render[j++] = ' ';
            while (j % TAB_STOP != 0)
                row->render[j++] = ' ';
        } else {
            row->render[j++] = row->chars[i];
        }
    }
    row->render[j] = '\0';
    row->rsize = j;
    E.dirty++;
}

void editorInsertRow(int at, char *line, size_t len) {
    E.rows = realloc(E.rows, sizeof(struct erow) * (E.numrows + 1));
    memmove(&E.rows[at + 1], &E.rows[at],
            sizeof(struct erow) * (E.numrows - at));
    E.rows[at].size = len;
    E.rows[at].chars = malloc(len + 1);
    memcpy(E.rows[at].chars, line, len);
    E.rows[at].chars[len] = '\0';

    E.rows[at].rsize = 0;
    E.rows[at].render = NULL;
    editorUpdateRow(&E.rows[at]);

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
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
            linelen--;
        }
        editorInsertRow(E.numrows, line, linelen);
    }

    free(line);
    fclose(fp);

    free(E.filename);
    E.filename = strndup(filename, 80);
    E.dirty = 0;
    editorSetStatusMessage("Opened %s", E.filename);
}

char *editorTextToString(size_t *len) {
    int buflen = 0;
    for (int i = 0; i < E.numrows; i++) {
        buflen += E.rows[i].size + 1;
    }
    *len = buflen;

    char *str = malloc(*len);
    char *p = str;
    for (int i = 0; i < E.numrows; i++, p++) {
        memcpy(p, E.rows[i].chars, E.rows[i].size);
        p += E.rows[i].size;
        *p = '\n';
    }
    return str;
}

void writeFile() {
    if (E.filename == NULL)
        return;
    size_t len;
    char *str = editorTextToString(&len);
    FILE *fp = fopen(E.filename, "w");
    if (fp == NULL) {
        editorSetStatusMessage("fopen failed");
        return;
    }
    if (fwrite(str, len, 1, fp) < 1) {
        editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
    }
    fclose(fp);
    free(str);
    E.dirty = 0;
    editorSetStatusMessage("%d bytes written to disk", len);
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
