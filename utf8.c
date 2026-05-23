#include "utf8.h"

int utf8CharLen(unsigned char c) {

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

int utf8NextChar(
    const char *s,
    int len,
    int i
) {

    if (i >= len)
        return len;

    return i +
        utf8CharLen(
            (unsigned char)s[i]
        );
}

int utf8PrevChar(
    const char *s,
    int i
) {

    if (i <= 0)
        return 0;

    i--;

    while (
        i > 0 &&
        ((unsigned char)s[i] & 0xC0) == 0x80
    ) {

        i--;
    }

    return i;
}
