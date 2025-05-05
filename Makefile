# 围棋游戏 Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I"C:\msys64\mingw64\include\SDL2" 
LDFLAGS = -L"C:\msys64\mingw64\lib" -lSDL2 -lSDL2_ttf -lm 

# 目标文件
TARGET = go_game

# 源文件
SRCS = main.c go_game.c go_ui.c

# 目标文件
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# 编译
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	del /Q $(TARGET).exe $(OBJS)

# 运行
run: $(TARGET)
	./$(TARGET)

# 依赖关系
main.o: main.c go_game.h go_ui.h
go_game.o: go_game.c go_game.h
go_ui.o: go_ui.c go_ui.h go_game.h

.PHONY: all clean run