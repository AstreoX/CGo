#ifndef GO_UI_H
#define GO_UI_H

#include "go_game.h"

// 游戏运行状态标志
extern bool isRunning;

// 初始化图形界面
void initUI(GameState* game);

// 清理图形界面资源
void cleanupUI();

// 绘制棋盘
void drawBoard(GameState* game);

// 绘制棋子
void drawStone(int x, int y, StoneColor color);

// 处理用户输入
void handleInput(GameState* game);

// 显示游戏状态信息
void displayGameInfo(GameState* game);

// 显示提示信息
void showMessage(const char* message);

// 处理游戏循环中的UI更新
void updateGameUI(GameState* game);

#endif // GO_UI_H