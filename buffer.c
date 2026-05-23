#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>

#include "editor.h"
#include "buffer.h"
#include "utf8.h"

char *C_HL_keywords[] = {
    "switch",
    "if",
    "while",
    "for",
    "break",
    "continue",
    "return",
    "else",
    "struct",
    "union",
    "typedef",
    "static",
    "enum",
    "class",
    "case",
    NULL
};

int is_separator(int c) {

    return isspace(c) ||
           c == '\0' ||
           strchr(",.()+-/*=~%<>[];",
                  c) != NULL;
}

int editorSyntaxToColor(int hl) {

    switch (hl) {

        case HL_COMMENT:
            return 36;

        case HL_KEYWORD1:
            return 33;

        case HL_STRING:
            return 35;

        case HL_NUMBER:
            return 31;

        default:
            return 37;
    }
}

void editorUpdateSyntax(EditorRow *row) {

    row->hl = realloc(row->hl,
                      row->rsize);

    memset(row->hl,
           HL_NORMAL,
           row->rsize);

    if (row->rsize == 0)
        return;

    int prev_sep = 1;

    int in_string = 0;

    for (int i = 0; i < row->rsize; i++) {

        char c = row->render[i];

        unsigned char prev_hl =
            (i > 0) ? row->hl[i - 1]
                    : HL_NORMAL;

        if (in_string) {

            row->hl[i] = HL_STRING;

            if (c == in_string)
                in_string = 0;

            prev_sep = 1;

            continue;
        }

        if (c == '"' || c == '\'') {

            in_string = c;

            row->hl[i] = HL_STRING;

            continue;
        }

        if (isdigit(c) &&
            (prev_sep ||
             prev_hl == HL_NUMBER)) {

            row->hl[i] = HL_NUMBER;

            prev_sep = 0;

            continue;
        }

        if (prev_sep) {

            int j;

            for (j = 0;
                 C_HL_keywords[j];
                 j++) {

                int klen =
                    strlen(C_HL_keywords[j]);

                if (!strncmp(&row->render[i],
                             C_HL_keywords[j],
                             klen) &&
                    is_separator(
                        row->render[i + klen])) {

                    memset(&row->hl[i],
                           HL_KEYWORD1,
                           klen);

                    i += klen - 1;

                    break;
                }
            }

            if (C_HL_keywords[j] != NULL) {

                prev_sep = 0;

                continue;
            }
        }

        prev_sep = is_separator(c);
    }
}

void editorUpdateRow(EditorRow *row) {

    int tabs = 0;

    for (int j = 0;
         j < row->size;
         j++)

        if (row->chars[j] == '\t')
            tabs++;

    free(row->render);

    row->render =
        malloc(row->size + tabs * 7 + 1);

    int idx = 0;

    for (int j = 0;
         j < row->size;
         j++) {

        if (row->chars[j] == '\t') {

            row->render[idx++] = ' ';

            while (idx % 8 != 0)
                row->render[idx++] = ' ';

        } else {

            row->render[idx++] =
                row->chars[j];
        }
    }

    row->render[idx] = '\0';

    row->rsize = idx;

    editorUpdateSyntax(row);
}

int utf8CharSize(unsigned char c) {

    if ((c & 0x80) == 0)
        return 1;

    if ((c & 0xE0) == 0xC0)
        return 2;

    if ((c & 0xF0) == 0xE0)
        return 3;

    if ((c & 0xF8) == 0xF0)
        return 4;

    return 1;
}

void editorInsertRow(int at,
                     char *s,
                     size_t len) {

    if (at < 0 || at > E.numrows)
        return;

    E.row = realloc(E.row,
                    sizeof(EditorRow) *
                    (E.numrows + 1));

    memmove(&E.row[at + 1],
            &E.row[at],
            sizeof(EditorRow) *
            (E.numrows - at));

    E.row[at].size = len;

    E.row[at].chars = malloc(len + 1);

    memcpy(E.row[at].chars, s, len);

    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;

    editorUpdateRow(&E.row[at]);

    E.numrows++;
}

void editorDelRow(int at) {

    if (at < 0 || at >= E.numrows)
        return;

    editorFreeRow(&E.row[at]);

    memmove(&E.row[at],
            &E.row[at + 1],
            sizeof(EditorRow) *
            (E.numrows - at - 1));

    E.numrows--;

    E.dirty++;
}

void editorFreeRow(EditorRow *row) {

    free(row->render);

    free(row->chars);

    free(row->hl);
}

void editorRowAppendString(EditorRow *row,
                           char *s,
                           size_t len) {

    row->chars =
        realloc(row->chars,
                row->size + len + 1);

    memcpy(&row->chars[row->size],
           s,
           len);

    row->size += len;

    row->chars[row->size] = '\0';

    editorUpdateRow(row);

    E.dirty++;
}

void editorRowInsertChar(EditorRow *row,
                         int at,
                         int c) {

    if (at < 0 || at > row->size)
        at = row->size;

    row->chars =
        realloc(row->chars,
                row->size + 2);

    memmove(&row->chars[at + 1],
            &row->chars[at],
            row->size - at + 1);

    row->size++;

    row->chars[at] = c;

    editorUpdateRow(row);
}

void editorRowInsertBytes(
    EditorRow *row,
    int at,
    const char *s,
    int len
) {

    if (at < 0 || at > row->size)
        at = row->size;

    row->chars =
        realloc(
            row->chars,
            row->size + len + 1
        );

    memmove(
        &row->chars[at + len],
        &row->chars[at],
        row->size - at + 1
    );

    memcpy(
        &row->chars[at],
        s,
        len
    );

    row->size += len;

    editorUpdateRow(row);
}

void editorRowDelChar(EditorRow *row,
                      int at) {

    if (at < 0 || at >= row->size)
        return;

    memmove(&row->chars[at],
            &row->chars[at + 1],
            row->size - at);

    row->size--;

    editorUpdateRow(row);
}

char *editorRowsToString(int *buflen) {

    int totlen = 0;

    for (int j = 0;
         j < E.numrows;
         j++)

        totlen += E.row[j].size + 1;

    *buflen = totlen;

    char *buf = malloc(totlen);

    char *p = buf;

    for (int j = 0;
         j < E.numrows;
         j++) {

        memcpy(p,
               E.row[j].chars,
               E.row[j].size);

        p += E.row[j].size;

        *p = '\n';

        p++;
    }

    return buf;
}
