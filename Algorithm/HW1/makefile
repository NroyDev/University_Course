CC		= g++
CFLAG	= -O2
CFLAG	+= -Wall -Wextra
SOURCES	= ./main.cpp
OBJECTS	= $(SOURCES:.cpp=.o)
TARGET	= ./tsp.exe

.PHONY: all
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAG) $^ -o $@
%.o: %.cpp
	$(CC) $(CFLAG) -c $< -o $@

.PHONY: clean
clean:
	rm $(TARGET) $(OBJECTS)
	rm ./fig.png ./ans.txt