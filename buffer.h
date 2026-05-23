#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct {

    int size;
    char *chars;

    int rsize;
    char *render;

    unsigned char *hl;

} EditorRow;

enum editorHighlight {

    HL_NORMAL = 0,
    HL_COMMENT,
    HL_KEYWORD1,
    HL_STRING,
    HL_NUMBER
};

void editorInsertRow(int at, char *s, size_t len);

void editorFreeRow(EditorRow *row);

void editorRowInsertChar(EditorRow *row,
                         int at,
                         int c);

void editorRowDelChar(EditorRow *row,
                      int at);

void editorRowInsertBytes(
    EditorRow *row,
    int at,
    const char *s,
    int len);

char *editorRowsToString(int *buflen);

void editorUpdateRow(EditorRow *row);

void editorUpdateSyntax(EditorRow *row);

int editorSyntaxToColor(int hl);

void editorDelRow(int at);

void editorRowAppendString(EditorRow *row,
                           char *s,
                           size_t len);

int utf8CharSize(unsigned char c);

int utf8PrevChar(const char *s, int index);

int utf8NextChar(const char *s, int len, int index);

#endif
