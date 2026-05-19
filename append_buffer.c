#include <stdlib.h>
#include <string.h>

#include "append_buffer.h"

void abAppend(AppendBuffer *ab, const char *s, int len) {

    char *new_buffer = realloc(ab->b, ab->len + len);

    if (new_buffer == NULL)
        return;

    memcpy(&new_buffer[ab->len], s, len);

    ab->b = new_buffer;

    ab->len += len;
}

void abFree(AppendBuffer *ab) {
    free(ab->b);
}
