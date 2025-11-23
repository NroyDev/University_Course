CC		= g++
CFLAG	= -O2
CFLAG	+= -Wall -Wextra
HEADERS = ./ACO.h
SOURCES	= ./ACO.cpp ./main.cpp
OBJECTS	= $(SOURCES:.cpp=.o)
TARGET	= ./tsp.exe

.PHONY: all
all: $(TARGET)
$(TARGET): $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAG) $^ -o $@
%.o: %.cpp
	$(CC) $(CFLAG) -c $< -o $@

.PHONY: clean
clean:
	rm $(TARGET) $(OBJECTS)
	rm ./fig.png ./ans.txt