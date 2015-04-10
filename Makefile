CC=gcc
CFLAGS=-std=c99
RM=rm -rf
OUT=schedule

all: build

build: main.o
	$(CC) $(CFLAGS) -o $(OUT) main.c
	$(RM) *.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) *.o $(OUT)
	
