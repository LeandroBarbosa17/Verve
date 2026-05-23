#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>

#include "editor.h"
#include "terminal.h"
#include "append_buffer.h"
#include "buffer.h"
#include "utf8.h"

#define CTRL_KEY(k) ((k) & 0x1f)

EditorConfig E;

enum EditorKey {
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

enum EditorActionType {
    ACTION_INSERT,
    ACTION_DELETE,
    ACTION_NEWLINE,
    ACTION_DEL_NEWLINE
};

int editorRowCxtoRx(EditorRow *row, int cx) {

    int rx = 0;
    int i = 0;

    while (i < cx) {

        if ((row->chars[i] & 0xC0) != 0x80)
            rx++;

        i++;
    }

    return rx;
    
}

void editorScroll() {

    if (E.cy < E.numrows)
        E.rx = editorRowCxtoRx(&E.row[E.cy], E.cx);
    else
        E.rx = 0;

    if (E.cy < E.rowoff)
        E.rowoff = E.cy;

    if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;

    if (E.rx < E.coloff)
        E.coloff = E.rx;

    if (E.rx >= E.coloff + E.screencols)
        E.coloff = E.rx - E.screencols + 1;
}

void editorDrawStatusBar(AppendBuffer *ab) {

    abAppend(ab, "\x1b[7m", 4);

    char status[80];
    char rstatus[80];

    int len = snprintf(status,
                       sizeof(status),
                       "%.20s - %d lines %s",
                       E.filename ? E.filename : "[No Name]",
                       E.numrows,
                       E.dirty ? "(modified)" : "");

    int rlen = snprintf(rstatus,
                        sizeof(rstatus),
                        "%d/%d",
                        E.cy + 1,
                        E.numrows);

    if (len > E.screencols)
        len = E.screencols;

    abAppend(ab, status, len);

    while (len < E.screencols) {

        if (E.screencols - len == rlen) {

            abAppend(ab, rstatus, rlen);

            break;

        } else {

            abAppend(ab, " ", 1);

            len++;
        }
    }

    abAppend(ab, "\x1b[m", 3);

    abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(AppendBuffer *ab) {

    abAppend(ab, "\x1b[K", 3);

    int msglen = strlen(E.statusmsg);

    if (msglen > E.screencols)
        msglen = E.screencols;

    if (msglen &&
        time(NULL) - E.statusmsg_time < 5)

        abAppend(ab, E.statusmsg, msglen);
}

int editorInSelection(int x, int y) {

    if (!E.selecting)
        return 0;

    int sx = E.sel_sx;
    int sy = E.sel_sy;

    int ex = E.sel_ex;
    int ey = E.sel_ey;

    if (sy > ey ||
       (sy == ey && sx > ex)) {

        int tx = sx;
        int ty = sy;

        sx = ex;
        sy = ey;

        ex = tx;
        ey = ty;
    }

    if (y < sy || y > ey)
        return 0;

    if (sy == ey) {

        return x >= sx &&
               x < ex;
    }

    if (y == sy)
        return x >= sx;

    if (y == ey)
        return x < ex;

    return 1;
}

void editorDrawRows(AppendBuffer *ab) {

    for (int y = 0; y < E.screenrows; y++) {

        int filerow = y + E.rowoff;

        if (filerow >= E.numrows) {

            abAppend(ab, "~", 1);

        } else {

            int len =
                E.row[filerow].rsize - E.coloff;

            if (len < 0)
                len = 0;

            if (len > E.screencols)
                len = E.screencols;

            char *c =
                &E.row[filerow]
                     .render[E.coloff];

            unsigned char *hl =
                &E.row[filerow]
                     .hl[E.coloff];

            int current_color = -1;

            int j = 0;

            while (j < len) {

                unsigned char ch = c[j];

                int char_len = 1;

                if ((ch & 0x80) == 0x00)
                    char_len = 1;
                else if ((ch & 0xE0) == 0xC0)
                    char_len = 2;
                else if ((ch & 0xF0) == 0xE0)
                    char_len = 3;
                else if ((ch & 0xF8) == 0xF0)
                    char_len = 4;

                int selected = editorInSelection(j + E.coloff, filerow);

                if (selected) {

                    abAppend(ab, "\x1b[7m", 4);
                    
                } else {

                    abAppend(ab, "\x1b[m", 3);
                    
                }

                if (hl[j] == HL_NORMAL) {

                    if (current_color != -1) {

                        abAppend(ab, "\x1b[39m", 5);

                        current_color = -1;
                    }

                    abAppend(ab, &c[j], char_len);
                    
                } else {

                    int color = editorSyntaxToColor(hl[j]);

                    if (color != current_color) {

                        current_color = color;

                        char buf[16];

                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);

                        abAppend(ab, buf, clen);
                    }

                    abAppend(ab, &c[j], char_len);
                }

                j += char_len;
            }

            abAppend(ab,
                     "\x1b[39m",
                     5);
        }

        abAppend(ab, "\x1b[K", 3);

        abAppend(ab, "\r\n", 2);
    }
}

void editorRefreshScreen() {

    editorScroll();

    AppendBuffer ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);

    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);

    editorDrawStatusBar(&ab);

    editorDrawMessageBar(&ab);

    char buf[32];

    snprintf(buf,
             sizeof(buf),
             "\x1b[%d;%dH",
             (E.cy - E.rowoff) + 1,
             (E.rx - E.coloff) + 1);

    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);

    abFree(&ab);
}

void editorPushUndo(EditorAction action) {

    if (E.undo_len == E.undo_cap) {

        E.undo_cap =
            E.undo_cap ? E.undo_cap * 2 : 64;

        E.undo =
            realloc(E.undo,
                    sizeof(EditorAction) *
                    E.undo_cap);
    }

    E.undo[E.undo_len++] = action;
}

void editorPushRedo(EditorAction action) {

    if (E.redo_len == E.redo_cap) {

        E.redo_cap =
            E.redo_cap ? E.redo_cap * 2 : 64;

        E.redo =
            realloc(E.redo,
                    sizeof(EditorAction) *
                    E.redo_cap);
    }

    E.redo[E.redo_len++] = action;
}

void editorClearRedo() {

    for (int i = 0;
         i < E.redo_len;
         i++) {

        free(E.redo[i].data);
    }

    E.redo_len = 0;
}

void editorInsertChar(int c)
{
    if (E.cy == E.numrows)
        editorInsertRow(E.numrows, "", 0);

    editorRowInsertChar(
        &E.row[E.cy],
        E.cx,
        c
    );

    E.cx++;

    E.dirty++;
}

int editorRowIndent(EditorRow *row) {

    int i = 0;

    while (i < row->size &&
          (row->chars[i] == ' ' ||
           row->chars[i] == '\t')) {

        i++;
    }

    return i;
}

void editorInsertNewline() {

    EditorAction action = {
        ACTION_NEWLINE,
        E.cx,
        E.cy,
        NULL
    };

    editorPushUndo(action);

    editorClearRedo();

    if (E.cx == 0) {

        editorInsertRow(E.cy,
                        "",
                        0);

    } else {

        EditorRow *row =
            &E.row[E.cy];

        editorInsertRow(
            E.cy + 1,
            &row->chars[E.cx],
            row->size - E.cx
        );

        row = &E.row[E.cy];

        row->size = E.cx;

        row->chars[row->size] = '\0';

        editorUpdateRow(row);
    }

    E.cy++;
    E.cx = 0;

    if (E.cy > 0) {

        EditorRow *prev =
            &E.row[E.cy - 1];

        int indent =
            editorRowIndent(prev);

        for (int i = 0;
             i < indent;
             i++) {

            editorInsertChar(
                prev->chars[i]);
        }

        if (prev->size > 0 &&
            prev->chars[prev->size - 1] == '{') {

            editorInsertChar(' ');
            editorInsertChar(' ');
            editorInsertChar(' ');
            editorInsertChar(' ');
        }
    }

    E.dirty++;
}

void editorDelChar() {

    if (E.cy == E.numrows)
        return;

    if (E.cx == 0 &&
        E.cy == 0)

        return;

    EditorRow *row =
        &E.row[E.cy];

    if (E.cx > 0) {

        if (E.cx >= 4) {

            int spaces = 1;

            for (int i = E.cx - 4;
                 i < E.cx;
                 i++) {

                if (row->chars[i] != ' ') {
                    spaces = 0;
                    break;
                }
            }

            if (spaces) {

                for (int i = 0;
                     i < 4;
                     i++) {

                        int prev =
                            utf8PrevChar(
                                row->chars,
                                E.cx
                            );

                        int bytes =
                            E.cx - prev;

                        memmove(
                            &row->chars[prev],
                            &row->chars[E.cx],
                            row->size - E.cx + 1
                        );

                        row->size -= bytes;

                        editorUpdateRow(row);

                        E.cx = prev;
                    
                }

                E.dirty++;

                return;
            }
        }
      
        char deleted =
          row->chars[E.cx - 1];

        EditorAction action = {
          ACTION_DELETE,
          E.cx - 1,
          E.cy,
          malloc(2)
        };

        action.data[0] = deleted;
        action.data[1] = '\0';

        editorPushUndo(action);

        editorClearRedo();

        int prev =
            utf8PrevChar(
                row->chars,
                E.cx
            );

        int bytes = E.cx - prev;

        memmove(
            &row->chars[prev],
            &row->chars[E.cx],
            row->size - E.cx + 1
        );

        row->size -= bytes;

        editorUpdateRow(row);

        E.cx = prev;       

    } else {

        E.cx =
            E.row[E.cy - 1].size;

        editorRowAppendString(
            &E.row[E.cy - 1],
            row->chars,
            row->size);

        editorDelRow(E.cy);

        E.cy--;
    }

    E.dirty++;
}

void editorMoveCursor(int key) {

    EditorRow *row =
        (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {

        case ARROW_LEFT:

            if (E.cx != 0) {

                E.cx =
                    utf8PrevChar(
                        row->chars,
                        E.cx
                    );

            }

            break;

        case ARROW_RIGHT:

            if (row && E.cx < row->size)

                E.cx =
                    utf8NextChar(
                        row->chars,
                        row->size,
                        E.cx
                    );
            break;

        case ARROW_UP:

            if (E.cy != 0)
                E.cy--;

            break;

        case ARROW_DOWN:

            if (E.cy < E.numrows)
                E.cy++;

            break;
    }

    row =
        (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    int rowlen = row ? row->size : 0;

    if (E.cx > rowlen)
        E.cx = rowlen;

    if (E.selecting) {

        E.sel_ex = E.cx;
        E.sel_ey = E.cy;
    }
}

void editorOpen(char *filename) {

    free(E.filename);

    E.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");

    if (!fp)
        return;

    char *line = NULL;

    size_t linecap = 0;

    ssize_t linelen;

    while ((linelen =
            getline(&line,
                    &linecap,
                    fp)) != -1) {

        while (linelen > 0 &&
               (line[linelen - 1] == '\n' ||
                line[linelen - 1] == '\r'))

            linelen--;

        editorInsertRow(E.numrows,
                        line,
                        linelen);
    }

    free(line);

    fclose(fp);

    E.dirty = 0;
}

void editorSave() {

    if (E.filename == NULL)
        return;

    int len;

    char *buf = editorRowsToString(&len);

    int fd = open(E.filename,
                  O_RDWR | O_CREAT,
                  0644);

    if (fd != -1) {

        if (ftruncate(fd, len) != -1) {

            if (write(fd, buf, len) == len) {

                close(fd);

                free(buf);

                E.dirty = 0;

                editorSetStatusMessage(
                    "%d bytes written to disk",
                    len);

                return;
            }
        }

        close(fd);
    }

    free(buf);

    editorSetStatusMessage(
        "Can't save! I/O error");
}

void editorSetStatusMessage(const char *fmt, ...) {

    va_list ap;

    va_start(ap, fmt);

    vsnprintf(E.statusmsg,
              sizeof(E.statusmsg),
              fmt,
              ap);

    va_end(ap);

    E.statusmsg_time = time(NULL);
}

char *editorPrompt(char *prompt) {

    size_t bufsize = 128;

    char *buf = malloc(bufsize);

    size_t buflen = 0;

    buf[0] = '\0';

    while (1) {

        editorSetStatusMessage(prompt, buf);

        editorRefreshScreen();

        int c = editorReadKey();

        if (c == '\r') {

            if (buflen != 0) {

                editorSetStatusMessage("");

                return buf;
            }

        } else if (c == 127 ||
                   c == CTRL_KEY('h')) {

            if (buflen != 0)
                buf[--buflen] = '\0';

        } else if (c == '\x1b') {

            editorSetStatusMessage("");

            free(buf);

            return NULL;

        } else if (!iscntrl(c) &&
                   c < 128) {

            if (buflen == bufsize - 1) {

                bufsize *= 2;

                buf = realloc(buf,
                              bufsize);
            }

            buf[buflen++] = c;

            buf[buflen] = '\0';
        }
    }
}

void editorFind() {

    char *query =
        editorPrompt("Search: %s (ESC to cancel)");

    if (query == NULL)
        return;

    for (int i = 0; i < E.numrows; i++) {

        EditorRow *row = &E.row[i];

        char *match =
            strstr(row->render,
                   query);

        if (match) {

            E.cy = i;

            E.cx =
                match - row->render;

            E.rowoff = E.numrows;

            break;
        }
    }

    free(query);
}

void editorUndo() {

    if (E.undo_len == 0)
        return;

    EditorAction action =
        E.undo[--E.undo_len];

    switch (action.type) {

        case ACTION_INSERT:

            E.cx = action.x + 1;
            E.cy = action.y;

            editorDelChar();

            break;

        case ACTION_DELETE:

            E.cx = action.x;
            E.cy = action.y;

            editorInsertChar(
                action.data[0]);

            E.cx = action.x + 1;

            break;

        case ACTION_NEWLINE:

            E.cy = action.y + 1;
            E.cx = 0;

            editorDelChar();

            break;
    }

    editorPushRedo(action);
}

void editorRedo() {

    if (E.redo_len == 0)
        return;

    EditorAction action =
        E.redo[--E.redo_len];

    switch (action.type) {

        case ACTION_INSERT:

            E.cx = action.x;
            E.cy = action.y;

            editorInsertChar(
                action.data[0]);

            break;

        case ACTION_DELETE:

            E.cx = action.x + 1;
            E.cy = action.y;

            editorDelChar();

            break;

        case ACTION_NEWLINE:

            E.cx = action.x;
            E.cy = action.y;

            editorInsertNewline();

            break;
    }

    editorPushUndo(action);
}

void editorStartSelection() {

    E.selecting = 1;

    E.sel_sx = E.cx;
    E.sel_sy = E.cy;

    E.sel_ex = E.cx;
    E.sel_ey = E.cy;
}

void editorNormalizeSelection(
    int *sx,
    int *sy,
    int *ex,
    int *ey
) {

    *sx = E.sel_sx;
    *sy = E.sel_sy;

    *ex = E.sel_ex;
    *ey = E.sel_ey;

    if (*sy > *ey ||
       (*sy == *ey && *sx > *ex)) {

        int tx = *sx;
        int ty = *sy;

        *sx = *ex;
        *sy = *ey;

        *ex = tx;
        *ey = ty;
    }
}

void editorCopySelection() {

    if (!E.selecting)
        return;

    free(E.clipboard);

    E.clipboard = NULL;

    int sx = E.sel_sx;
    int sy = E.sel_sy;

    int ex = E.sel_ex;
    int ey = E.sel_ey;

    if (sy > ey ||
       (sy == ey && sx > ex)) {

        int tx = sx;
        int ty = sy;

        sx = ex;
        sy = ey;

        ex = tx;
        ey = ty;
    }

    size_t cap = 128;

    E.clipboard = malloc(cap);

    size_t len = 0;

    for (int y = sy; y <= ey; y++) {

        EditorRow *row = &E.row[y];

        int start =
            (y == sy) ? sx : 0;

        int end =
            (y == ey)
            ? ex
            : row->size;

        for (int i = start;
             i < end;
             i++) {

            if (len + 2 >= cap) {

                cap *= 2;

                E.clipboard =
                    realloc(E.clipboard,
                            cap);
            }

            E.clipboard[len++] =
                row->chars[i];
        }

        if (y != ey)
            E.clipboard[len++] = '\n';
    }

    E.clipboard[len] = '\0';
}

void editorPasteClipboard() {

    if (!E.clipboard)
        return;

    size_t len =
    strlen(E.clipboard);

    for (size_t i = 0;
         i < len;
         i++) {

        char c = E.clipboard[i];

        if (c == '\n') {

            editorInsertNewline();

        } else {

            editorInsertChar(c);
        }
    }
}

void editorDeleteSelection() {

    if (!E.selecting)
        return;

    int sx, sy, ex, ey;

    editorNormalizeSelection(
        &sx,
        &sy,
        &ex,
        &ey
    );

    if (sy == ey) {

        EditorRow *row =
            &E.row[sy];

        memmove(
            &row->chars[sx],
            &row->chars[ex],
            row->size - ex + 1
        );

        row->size -= (ex - sx);

        editorUpdateRow(row);

    } else {

        EditorRow *start =
            &E.row[sy];

        EditorRow *end =
            &E.row[ey];

        int tail_len =
            end->size - ex;

        char *tail =
            malloc(tail_len + 1);

        memcpy(
            tail,
            &end->chars[ex],
            tail_len
        );

        tail[tail_len] = '\0';

        start->size = sx;

        start->chars[sx] = '\0';

        start->chars =
            realloc(start->chars,
                    sx + tail_len + 1);

        memcpy(
            &start->chars[sx],
            tail,
            tail_len + 1
        );

        start->size += tail_len;

        free(tail);

        editorUpdateRow(start);

        for (int i = sy + 1;
             i <= ey;
             i++) {

            editorDelRow(sy + 1);
        }
    }

    E.cx = sx;
    E.cy = sy;

    E.selecting = 0;

    E.dirty++;
}

void editorCutSelection() {

    if (!E.selecting)
        return;

    editorCopySelection();

    editorDeleteSelection();
}

void editorProcessKeypress() {

    int c = editorReadKey();

    switch (c) {

        case CTRL_KEY('q'):

            write(STDOUT_FILENO,
                  "\x1b[2J",
                  4);

            write(STDOUT_FILENO,
                  "\x1b[H",
                  3);

            exit(0);

            break;

        case CTRL_KEY('s'):

            editorSave();

            break;

        case CTRL_KEY('f'):

            editorFind();

            break;

        case CTRL_KEY('z'):

            editorUndo();

            break;

        case CTRL_KEY('y'):

            editorRedo();

            break;
            
        case CTRL_KEY('b'):

            editorStartSelection();

            break;

        case CTRL_KEY('c'):

            editorCopySelection();

            E.selecting = 0;

            break;

        case CTRL_KEY('x'):

            editorCutSelection();

            break;

        case CTRL_KEY('v'):

            editorPasteClipboard();

            break;

        case '\x1b':

            E.selecting = 0;

            break;

        case '\r':

            editorInsertNewline();

            break;

        case '\t':

            for (int i = 0; i < 4; i++)
                editorInsertChar(' ');

            break;

        case BACKSPACE:

            editorDelChar();

            break;

        case HOME_KEY:

            E.cx = 0;

            break;

        case END_KEY:

            if (E.cy < E.numrows)
              E.cx = E.row[E.cy].size;

            break;

        case PAGE_UP:
        case PAGE_DOWN:
        {
          if (c == PAGE_UP) {

            E.cy = E.rowoff;

          } else if (c == PAGE_DOWN) {

            E.cy =
              E.rowoff +
              E.screenrows - 1;

            if (E.cy > E.numrows)
              E.cy = E.numrows;
          }

          int times = E.screenrows;

          while (times--)
            editorMoveCursor(
              c == PAGE_UP
              ? ARROW_UP
              : ARROW_DOWN);
        }
        break;

        case DEL_KEY:

          editorMoveCursor(ARROW_RIGHT);

          editorDelChar();

          break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:

            editorMoveCursor(c);

            break;

        default:

            if (!iscntrl(c))
                editorInsertChar(c);

            break;
    }
}

void initEditor() {

    E.cx = 0;
    E.cy = 0;

    E.rx = 0;

    E.rowoff = 0;
    E.coloff = 0;

    E.numrows = 0;

    E.row = NULL;

    E.filename = NULL;

    E.dirty = 0;

    E.undo = NULL;
    E.undo_len = 0;
    E.undo_cap = 0;

    E.redo = NULL;
    E.redo_len = 0;
    E.redo_cap = 0;

    if (getWindowSize(&E.screenrows,
                      &E.screencols) == -1)

        exit(1);

    E.screenrows -= 2;

    E.selecting = 0;

    E.clipboard = NULL;
}
