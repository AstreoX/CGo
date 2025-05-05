#ifndef GO_GAME_H
#define GO_GAME_H

#include <stdbool.h>

// 棋盘大小（标准19x19）
#define BOARD_SIZE 19

// 棋子颜色
typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} StoneColor;

// 棋子位置
typedef struct {
    int x;
    int y;
} Position;

// 落子记录节点（双向链表）
typedef struct MoveNode {
    Position pos;
    StoneColor color;
    struct MoveNode* prev;
    struct MoveNode* next;
} MoveNode;

// 游戏状态
typedef struct {
    StoneColor board[BOARD_SIZE][BOARD_SIZE];  // 棋盘
    StoneColor currentPlayer;                   // 当前玩家
    MoveNode* moveHistory;                      // 落子历史（链表头）
    MoveNode* currentMove;                      // 当前落子（链表尾）
    Position lastCapture;                       // 最后一次提子位置（用于劫争判断）
    bool isGameOver;                            // 游戏是否结束
    int blackCaptures;                          // 黑方提子数
    int whiteCaptures;                          // 白方提子数
} GameState;

// 游戏初始化
GameState* initGame();

// 清理游戏资源
void cleanupGame(GameState* game);

// 游戏主循环
void runGameLoop(GameState* game);

// 落子
bool placeStone(GameState* game, int x, int y);

// 检查落子是否合法
bool isValidMove(GameState* game, int x, int y);

// 检查劫争
bool isKo(GameState* game, int x, int y);

// 计算棋子气数
int countLiberties(GameState* game, int x, int y);

// 提子
int captureStones(GameState* game, int x, int y);

// 悔棋
bool undoMove(GameState* game);

// 重做
bool redoMove(GameState* game);

// 棋局回溯（复盘）
void replayGame(GameState* game);

// 切换玩家
void switchPlayer(GameState* game);

// AI落子
Position aiMakeMove(GameState* game);

#endif // GO_GAME_H