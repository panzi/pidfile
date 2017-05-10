CC = gcc
BIN = build/pidfile
OBJ = build/pidfile.o
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -g
PREFIX = /usr

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJ): pidfile.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -v $(OBJ) $(BIN)

install:
	install $(BIN) $(PREFIX)

uninstall:
	rm -v $(PREFIX)/bin/pidfile
