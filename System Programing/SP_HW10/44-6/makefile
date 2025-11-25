CC = gcc
CFLAG = -Wall -Wextra

all:
	$(CC) $(CFLAG) -c lib/error_functions.c
	$(CC) $(CFLAG) -c lib/get_num.c
	$(CC) $(CFLAG) -o server get_num.o error_functions.o server.c
	$(CC) $(CFLAG) -o server_revised get_num.o error_functions.o server_revised.c
	$(CC) $(CFLAG) -o client_normal get_num.o error_functions.o client_normal.c
	$(CC) $(CFLAG) -o client_attack get_num.o error_functions.o client_attack.c

clean:
	rm get_num.o error_functions.o server server_revised client_normal client_attack
	rm ./tmp/*