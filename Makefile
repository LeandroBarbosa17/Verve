CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c99

SRC = main.c terminal.c editor.c buffer.c append_buffer.c utf8.c

OUT = verve

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
