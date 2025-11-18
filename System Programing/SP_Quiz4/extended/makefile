CC		= gcc
CFLAG	= -Wall -Wextra -lrt -pthread

all:
	$(CC) $(CFLAG) -o data_aggregator_mutex data_aggregator_mutex.c
	$(CC) $(CFLAG) -o data_aggregator_semaphore data_aggregator_semaphore.c

clean:
	rm ./data_aggregator_mutex ./data_aggregator_semaphore