#ifndef EDITOR_H
#define EDITOR_H

#include <time.h>

#include "buffer.h"

typedef struct {

    int type;

    int x;
    int y;

    char *data;

} EditorAction;

typedef struct {

    int cx;
    int cy;

    int rx;

    int rowoff;
    int coloff;

    int screenrows;
    int screencols;

    int numrows;

    EditorRow *row;

    char *filename;

    int dirty;

    char statusmsg[80];
    time_t statusmsg_time;

    EditorAction *undo;

    int undo_len;
    int undo_cap;

    EditorAction *redo;

    int redo_len;
    int redo_cap;

    int selecting;

    int sel_sx;
    int sel_sy;

    int sel_ex;
    int sel_ey;

    char *clipboard;

} EditorConfig;

void editorRefreshScreen();

void editorProcessKeypress();

void initEditor();

void editorSetStatusMessage(const char *fmt, ...);

void editorOpen(char *filename);

void editorSave();

char *editorPrompt(char *prompt);

void editorFind();

void editorUndo();

void editorRedo();

extern EditorConfig E;

#endif
