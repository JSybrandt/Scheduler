CC=gcc
CFLAGS=-std=c99
RM=rm -rf
OUT=schedule

all: build

build: ScheduleSim.o
	$(CC) $(CFLAGS) -o $(OUT) ScheduleSim.c
	$(RM) *.o

ScheduleSim.o: ScheduleSim.c
	$(CC) $(CFLAGS) -c ScheduleSim.c

clean:
	$(RM) *.o $(OUT)
	
