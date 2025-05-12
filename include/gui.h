/**
 * @file gui.h
 * @brief 围棋游戏图形界面
 */

#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "game.h"

// 窗口尺寸
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// 棋盘尺寸和位置
#define BOARD_MARGIN 50
#define CELL_SIZE 26

// 界面结构
typedef struct {
    SDL_Window* window;       // SDL窗口
    SDL_Renderer* renderer;   // SDL渲染器
    SDL_Texture* logoTexture; // 国防科技大学图样纹理
    SDL_Texture* backgroundTexture; // 背景图片纹理
    SDL_Rect logoRect;        // 图样位置和大小
    SDL_Rect boardRect;       // 棋盘位置和大小
    SDL_Rect statusRect;      // 状态栏位置和大小
    SDL_Rect violationRect;   // 违规提示位置和大小
    SDL_Rect controlsRect;    // 控制提示位置和大小
} GUI;

/**
 * @brief 初始化图形界面
 * @param gui GUI指针
 * @return 初始化是否成功
 */
bool initGUI(GUI* gui);

/**
 * @brief 释放图形界面资源
 * @param gui GUI指针
 */
void freeGUI(GUI* gui);

/**
 * @brief 渲染游戏界面
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderGame(GUI* gui, Game* game);

/**
 * @brief 渲染棋盘
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderBoard(GUI* gui, Game* game);

/**
 * @brief 渲染状态信息
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderStatus(GUI* gui, Game* game);

/**
 * @brief 渲染违规提示
 * @param gui GUI指针
 * @param game 游戏指针
 * @param message 提示信息
 */
void renderViolationHint(GUI* gui, Game* game, const char* message);

/**
 * @brief 渲染控制提示
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderControls(GUI* gui, Game* game);

/**
 * @brief 渲染游戏结束界面
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderGameOver(GUI* gui, Game* game);

/**
 * @brief 将屏幕坐标转换为棋盘坐标
 * @param gui GUI指针
 * @param screenX 屏幕X坐标
 * @param screenY 屏幕Y坐标
 * @param boardPos 棋盘坐标（输出）
 * @return 是否在棋盘范围内
 */
bool screenToBoardPos(GUI* gui, int screenX, int screenY, Position* boardPos);

/**
 * @brief 处理SDL事件
 * @param gui GUI指针
 * @param game 游戏指针
 * @return 是否继续游戏
 */
bool handleEvents(GUI* gui, Game* game);

#endif // GUI_H