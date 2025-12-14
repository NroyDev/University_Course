CC = gcc
CFLAGS = -Wall -Wextra

all:
	$(CC) $(CFLAGS) -o udp_server dict.h lookup5.c udp_server.c 
	$(CC) $(CFLAGS) -o udp_client dict.h lookup9.c main.c

clean:
	rm ./udp_client ./udp_server