CC = gcc
CFLAGS	= -O2
CFLAGS	+= -Wall -Wextra 
HEADERS	= shell.h
SOURCES	= builtin.c is_background.c parse.c run_command.c shell.c
OBJECTS	= $(SOURCES:.c=.o)
TARGET	= myshell

# ------------------------------------------------------------------

.PHONY: all
# .PHONY: all 是防止目錄下有 all 的執行檔 導致執行上發生問題(雖然不加這行不會有問題)
all: $(TARGET)
#	rm $(OBJECTS)

# $^: 上面所有的依賴  $@: 規則的目標=> $(TARGET)
$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $^ -o $@

# 規則模式: %.o 依賴 %.c => 定義如何透過.c產生.o
# $<: 第一個依賴
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
# .PHONY: clean 是防止目錄下有 clean 的執行檔 導致執行上發生問題(雖然不加這行不會有問題)
clean:
	- rm $(TARGET) $(OBJECTS)
