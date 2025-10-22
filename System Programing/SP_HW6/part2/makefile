CC		=	gcc
CFLAG	=	-O2
CFLAG	+=	-Wall -Wextra
HEADERS	=	$(wildcard *.h)
SOURCES	=	$(wildcard *.c)
OBJECTS	=	$(SOURCES:%.c=%.o)
TARGET	=	myshell

.PHONY: all
all: $(TARGET)
$(TARGET): $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAG) -o $@ $^
%.o:%.c
	$(CC) $(CFLAG) -c $<

.PHONY: clean
clean:
	-rm $(OBJECTS) $(TARGET)