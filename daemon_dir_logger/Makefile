CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = figger # file logger
SRC = main.c logger.c utils.c inotify_monitor.c fanotify_monitor.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c monitor.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
