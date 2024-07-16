#ifndef _MODES_H
#define _MODES_H

#include "main.h"

void editorRowInsertChar(struct erow *row, char c, int cx);
void editorRowDeleteChar(struct erow *row, int cx);
void editorAppendNewLine(void);
void editorSwitchNormalMode(void);

#endif
