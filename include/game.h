/**
 * @file game.h
 * @brief 围棋游戏逻辑和规则
 */

#ifndef GAME_H
#define GAME_H

#include "board.h"

// 游戏模式
typedef enum {
    MODE_PVP,    // 人人对战
    MODE_PVE     // 人机对战
} GameMode;

// 游戏状态
typedef enum {
    STATE_MENU,      // 菜单状态
    STATE_PLAYING,   // 游戏进行中
    STATE_GAMEOVER,  // 游戏结束
    STATE_QUIT       // 退出游戏
} GameState;

// 游戏结构
typedef struct Game {
    Board board;          // 棋盘
    GameMode mode;        // 游戏模式
    GameState state;      // 游戏状态
    bool showHints;       // 是否显示提示
    bool aiThinking;      // AI是否在思考中
    int winner;           // 胜者 (0=无, 1=黑, 2=白)
} Game;

/**
 * @brief 初始化游戏
 * @param game 游戏指针
 */
void initGame(Game* game);

/**
 * @brief 释放游戏资源
 * @param game 游戏指针
 */
void freeGame(Game* game);

/**
 * @brief 处理玩家落子
 * @param game 游戏指针
 * @param pos 落子位置
 * @return 落子是否成功
 */
bool handlePlayerMove(Game* game, Position pos);

/**
 * @brief 处理AI落子
 * @param game 游戏指针
 * @return 落子是否成功
 */
bool handleAIMove(Game* game);

/**
 * @brief 使用AI为当前玩家落子（无论黑白）
 * @param game 游戏指针
 * @return 落子是否成功
 */
bool makeAIMoveForCurrentPlayer(Game* game);

/**
 * @brief 切换游戏模式
 * @param game 游戏指针
 */
void toggleGameMode(Game* game);

/**
 * @brief 切换提示显示
 * @param game 游戏指针
 */
void toggleHints(Game* game);

/**
 * @brief 处理悔棋
 * @param game 游戏指针
 * @return 是否成功
 */
bool handleUndo(Game* game);

/**
 * @brief 处理前进（取消悔棋）
 * @param game 游戏指针
 * @return 是否成功
 */
bool handleRedo(Game* game);

/**
 * @brief 更新游戏状态
 * @param game 游戏指针
 */
void updateGame(Game* game);

/**
 * @brief 检查游戏是否结束
 * @param game 游戏指针
 * @return 游戏是否结束
 */
bool isGameOver(Game* game);

/**
 * @brief 手动结束游戏并计算胜负
 * @param game 游戏指针
 */
void endGameManually(Game* game);

/**
 * @brief 获取违规行为提示信息
 * @param game 游戏指针
 * @param pos 位置
 * @return 提示信息（如果没有违规则返回NULL）
 */
const char* getViolationHint(Game* game, Position pos);

#endif // GAME_H