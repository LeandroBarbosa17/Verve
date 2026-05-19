#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include "terminal.h"

static struct termios original_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enableRawMode() {

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    atexit(disableRawMode);

    struct termios raw = original_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    raw.c_oflag &= ~(OPOST);

    raw.c_cflag |= (CS8);

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

enum EditorKey {
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

int editorReadKey() {

    char c;

    while (read(STDIN_FILENO, &c, 1) != 1);

    if (c == '\x1b') {

        char seq[3];

        if (read(STDIN_FILENO,
                 &seq[0],
                 1) != 1)

            return '\x1b';

        if (read(STDIN_FILENO,
                 &seq[1],
                 1) != 1)

            return '\x1b';

        if (seq[0] == '[') {

            if (seq[1] >= '0' &&
                seq[1] <= '9') {

                if (read(STDIN_FILENO,
                         &seq[2],
                         1) != 1)

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
        }

        return '\x1b';
    }

    return c;
}

int getWindowSize(int *rows, int *cols) {

    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 ||
        ws.ws_col == 0) {

        return -1;
    }

    *cols = ws.ws_col;
    *rows = ws.ws_row;

    return 0;
}
