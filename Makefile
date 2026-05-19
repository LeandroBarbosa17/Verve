CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c99

SRC = main.c terminal.c editor.c buffer.c append_buffer.c

OUT = editor

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
