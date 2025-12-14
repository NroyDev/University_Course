CC = gcc
CFLAGS = -Wall -Wextra

all:
	$(CC) $(CFLAGS) -o isock_server dict.h lookup5.c isock_server.c 
	$(CC) $(CFLAGS) -o isock_client dict.h lookup8.c main.c

clean:
	rm ./isock_server ./isock_client