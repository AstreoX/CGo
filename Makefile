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
# DLL目录
LIBS_DIR = libs

# 源文件
SRCS = $(wildcard $(SRC_DIR)/*.c) main.c
# 目标文件
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))
# 需要的DLL文件
DLLS = SDL2.dll SDL2_image.dll SDL2_ttf.dll libfreetype-6.dll zlib1.dll libpng16-16.dll libjpeg-8.dll libtiff-5.dll libwebp-7.dll

# 默认目标
all: directories $(TARGET)
	@echo "编译完成: $(TARGET)"

# 创建必要的目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(INC_DIR)
	@mkdir -p resources
	@mkdir -p $(LIBS_DIR)

# 链接目标文件
$(TARGET): $(OBJS)
	@echo "正在链接: $(TARGET)"
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "编译: $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c
	@echo "编译: main.c"
	$(CC) $(CFLAGS) -c $< -o $@

# 复制所需的DLL文件到libs目录
copy_dlls:
	@echo "复制所需DLL文件到 $(LIBS_DIR) 目录..."
	@for dll in $(DLLS); do \
		cp C:/msys64/mingw64/bin/$$dll $(LIBS_DIR)/ 2>/dev/null || echo "警告: $$dll 不存在"; \
	done

# 清理
clean:
	@echo "清理构建文件..."
	rm -rf $(BUILD_DIR) $(TARGET)

# 清理DLL
clean_dlls:
	@echo "清理DLL文件..."
	rm -rf $(LIBS_DIR)

# 优雅运行方式1: 使用libs目录中的DLL (推荐)
run: all copy_dlls
	@echo "运行: $(TARGET) (使用libs目录中的DLL)"
	powershell -Command "$$env:PATH = '$(CURDIR)/$(LIBS_DIR);' + $$env:PATH; ./$(TARGET)"

# 优雅运行方式2: 使用系统路径中的DLL
run_with_system_path: all
	@echo "运行: $(TARGET) (使用系统路径中的DLL)"
	powershell -Command "$$env:PATH = 'C:/msys64/mingw64/bin;' + $$env:PATH; ./$(TARGET)"

# 仅使用必要的DLL，用于发布
prepare_release: all copy_dlls
	@echo "准备发布版本..."
	@mkdir -p release
	@cp $(TARGET) release/
	@cp $(LIBS_DIR)/*.dll release/
	@cp -r resources release/
	@echo "发布版本已准备好，位于release目录"

.PHONY: all clean clean_dlls run run_with_system_path copy_dlls prepare_release directories