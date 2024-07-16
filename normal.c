#include <stdlib.h>
#include <unistd.h>

#include "main.h"
#include "normal.h"
#include "normal.statics.h"

static struct Command {
    int key;
    void (*cmd)(void);
} normal_commands[] = {
    {CTRL_KEY('q'), quit},
    {CTRL_KEY('s'), writeFile},
    {'0', goToLineStart},
    {HOME_KEY, goToLineStart},
    {'$', goToLineEnd},
    {END_KEY, goToLineEnd},
    {'i', switchInsertMode},
    {'I', switchInsertModeStart},
    {'a', switchInsertModeAppend},
    {'A', switchInsertModeAppendEnd},
    {'x', editorDelCharUnderCursor},
    {'X', editorDelChar},
    {CTRL_KEY('k'), clearMessage},
    {'\r', newLine},
};

static int quit_times = 0;

static void quit(void) {
    if (E.dirty > 0 && quit_times == 0) {
        editorSetStatusMessage("Quit without saving? (C-q again to confirm)");
        quit_times++;
        return;
    }
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
}

static void clearMessage() { editorSetStatusMessage(""); }

static void newLine() {
    E.cy++;
    E.cx = 0;
}

static void goToLineStart(void) { E.cx = 0; }

static void goToLineEnd(void) {
    if (E.cy < E.numrows)
        E.cx = E.rows[E.cy].size;
}

static void switchInsertMode(void) { E.mode = INSERT; }

static void switchInsertModeStart(void) {
    goToLineStart();
    switchInsertMode();
}

static void switchInsertModeAppend(void) {
    editorMoveCursor(ARROW_RIGHT);
    switchInsertMode();
}

static void switchInsertModeAppendEnd(void) {
    goToLineEnd();
    switchInsertMode();
}

static struct Command *find_command(int c) {
    for (int i = 0; i < ARRAY_SIZE(normal_commands); i++) {
        if (normal_commands[i].key == c) {
            return &normal_commands[i];
        }
    }
    return NULL;
}

void executeNormalCommand(int c) {
    struct Command *cmd = find_command(c);
    if (cmd != NULL) {
        cmd->cmd();
        return;
    }

    switch (c) {
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

        /*case BACKSPACE:*/
        /*case DEL_KEY:*/
        /*case CTRL_KEY('h'):*/
        /*    if (c == DEL_KEY)*/
        /*        editorMoveCursor(ARROW_RIGHT);*/
        /*    editorDelChar();*/
        /*break;*/

    case CTRL_KEY('l'):
    case '\x1b':
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
    }

    quit_times = 0;
}
