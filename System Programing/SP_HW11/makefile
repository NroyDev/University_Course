CC		= gcc
CFLAG	= -lrt -pthread
# CFLAG	+= -Wall -Wextra

all:
	$(CC) $(CFLAG) -o program program.c
	$(CC) $(CFLAG) -o program_revised program_revised.c

clean:
	rm ./program program_revised