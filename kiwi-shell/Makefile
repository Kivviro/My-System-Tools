CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2

TARGET = kiwish

SRC = src/terminal.c

OBJ = $(SRC:.c=0.)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
