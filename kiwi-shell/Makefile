CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2

TARGET = ksh

SRC = src/main.c src/builtin.c src/completion.c src/executor.c src/history.c src/parser.c src/shell.c src/terminal.c

OBJ = $(SRC:.c=.o)

CPPFLAGS = -Iinclude

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
