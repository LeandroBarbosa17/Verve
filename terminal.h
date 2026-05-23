#ifndef TERMINAL_H
#define TERMINAL_H

void enableRawMode();
void disableRawMode();

int editorReadKey();

int getWindowSize(int *rows, int *cols);

int editorReadUtf8Char(char *buf);

#endif
