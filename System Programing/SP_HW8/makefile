CC = gcc
CFLAG = -Wall -Wextra

all:
	$(CC) $(CFLAG) -c error_functions.c
	$(CC) $(CFLAG) -c get_num.c
	$(CC) $(CFLAG) -o 22_3 get_num.o error_functions.o 22_3.c
	$(CC) $(CFLAG) -o 22_4 22_4.c

clean:
	rm get_num.o error_functions.o 22_3 22_4
