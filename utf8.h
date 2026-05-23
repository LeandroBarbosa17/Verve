#ifndef UTF8_H
#define UTF8_H

int utf8CharLen(unsigned char c);

int utf8NextChar(
    const char *s,
    int len,
    int i
);

int utf8PrevChar(
    const char *s,
    int i
);

#endif
