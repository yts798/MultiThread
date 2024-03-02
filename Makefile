CFLAGS=-Wall -Wno-comment -std=gnu99

allocate: allocate.o scheduler.o list.o
	gcc $(CFLAGS) -o allocate allocate.o scheduler.o list.o

allocate.o: allocate.c scheduler.h list.h
	gcc $(CFLAGS) -c allocate.c

scheduler.o: scheduler.c scheduler.h list.h
	gcc $(CFLAGS) -c scheduler.c

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c

.PHONY: clean

clean:
	rm -f
