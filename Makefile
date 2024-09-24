# Makefile

CC = gcc
CFLAGS = -Wall

all: oss worker


oss: oss.c
	$(CC) $(CFLAGS) -o oss oss.c

worker: worker.c
	$(CC) $(CFLAGS) -o worker worker.c

oss.o: oss.c
	$(CC) $(CFLAGS) -c oss.c

worker.o: worker.c
	$(CC) $(CFLAGS) -c worker.c

clean:
	rm -f oss worker


