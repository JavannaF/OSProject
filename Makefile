CC = gcc -Wall -O0 -g
LDFLAGS =

all: server client

server: main.c common.h
	$(CC) -o server main.c $(LDFLAGS)

client: client.c common.h
	$(CC) -o client client.c $(LDFLAGS)

:phony
clean:
	rm -f client server

