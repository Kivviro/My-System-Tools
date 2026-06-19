CC=gcc
CFLAGS=-std=c17
OBJ=main.o cpu.o memory.o tasks.o uptime.o

kmonitor.out: $(OBJ)
	$(CC) $(OBJ) -o kmonitor.out

main.o: main.c cpu.h memory.h
	$(CC) $(CFLAGS) -c main.c

cpu.o: cpu.c cpu.h
	$(CC) $(CFLAGS) -c cpu.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

tasks.o: tasks.c tasks.h
	$(CC) $(CFLAGS) -c tasks.c

uptime.o: tasks.c uptime.h
	$(CC) $(CFLAGS) -c uptime.c

clean:
	rm -f *.o kmonitor.out