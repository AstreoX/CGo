/**
 * @file utils.h
 * @brief 围棋游戏工具函数
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

// 错误处理宏
#define LOG_ERROR(msg) fprintf(stderr, "错误: %s (文件: %s, 行: %d)\n", msg, __FILE__, __LINE__)

// 资源路径
#define RESOURCES_DIR "resources"
#define LOGO_FILE "resources/LOGO.jpg"
#define BACKGROUND_FILE "resources/background.jpg"

/**
 * @brief 初始化随机数生成器
 */
void initRandom(void);

/**
 * @brief 生成指定范围内的随机数
 * @param min 最小值
 * @param max 最大值
 * @return 随机数
 */
int randomInt(int min, int max);

/**
 * @brief 检查文件是否存在
 * @param filename 文件名
 * @return 是否存在
 */
bool fileExists(const char* filename);

/**
 * @brief 创建目录（如果不存在）
 * @param dirname 目录名
 * @return 是否成功
 */
bool createDirectory(const char* dirname);

/**
 * @brief 获取当前时间字符串
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 */
void getCurrentTimeString(char* buffer, size_t size);

#endif // UTILS_H