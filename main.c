#include "terminal.h"
#include "editor.h"
#include <locale.h>

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");

    enableRawMode();

    initEditor();

    if (argc >= 2)
        editorOpen(argv[1]);

    editorSetStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-Q = quit");

    while (1) {

        editorRefreshScreen();

        editorProcessKeypress();
    }

    return 0;
}
