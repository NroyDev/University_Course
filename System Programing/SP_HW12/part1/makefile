CC = gcc
CFLAGS = -Wall -Wextra

all:
	$(CC) $(CFLAGS) -o usock_server dict.h lookup5.c usock_server.c 
	$(CC) $(CFLAGS) -o usock_client dict.h lookup7.c main.c

clean:
	rm ./usock_server ./usock_client ./TUNNEL