CC 		= 	g++
CFLAG 	= 	-O2
CFLAG	+=	-Wall -Wextra

all:
	$(CC) $(FLAG) -o convert dict.h convert.c
	$(CC) $(FLAG) -o file_lookup dict.h lookup1.c main.c

clean:
	rm convert file_lookup
