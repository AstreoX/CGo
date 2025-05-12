/**
 * @file utils.c
 * @brief 围棋游戏工具函数实现
 */

#include "../include/utils.h"
#include <sys/stat.h>
#include <direct.h>

void initRandom(void) {
    // 使用当前时间初始化随机数生成器
    srand((unsigned int)time(NULL));
}

int randomInt(int min, int max) {
    // 生成指定范围内的随机数
    return min + rand() % (max - min + 1);
}

bool fileExists(const char* filename) {
    if (!filename) return false;
    
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

bool createDirectory(const char* dirname) {
    if (!dirname) return false;
    
    // 在Windows下创建目录
    return (_mkdir(dirname) == 0 || errno == EEXIST);
}

void getCurrentTimeString(char* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}