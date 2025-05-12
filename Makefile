# 围棋游戏Makefile

CC = gcc # 编译器
CFLAGS = -Wall -Wextra -g -Iinclude -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\SDL2
LDFLAGS = -LC:\msys64\mingw64\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lm -mwindows

# 目标文件
TARGET = cgogame

# 源文件目录
SRC_DIR = src
# 头文件目录
INC_DIR = include
# 构建目录
BUILD_DIR = build

# 源文件
SRCS = $(wildcard $(SRC_DIR)/*.c) main.c
# 目标文件
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

# 默认目标
all: directories $(TARGET)

# 创建必要的目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(INC_DIR)
	@mkdir -p resources

# 链接目标文件
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# 运行
run: all
	./$(TARGET)

.PHONY: all clean run directories