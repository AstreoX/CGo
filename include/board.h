/**
 * @file board.h
 * @brief 围棋棋盘数据结构和基本操作
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// 棋盘大小
#define BOARD_SIZE 19

// 棋子颜色
typedef enum {
    EMPTY = 0,  // 空位
    BLACK = 1,  // 黑棋
    WHITE = 2   // 白棋
} Stone;

// 棋盘位置
typedef struct {
    int x;  // 横坐标 (0-18)
    int y;  // 纵坐标 (0-18)
} Position;

// 棋盘历史记录节点（双向链表）
typedef struct BoardHistoryNode {
    Stone board[BOARD_SIZE][BOARD_SIZE];  // 棋盘状态
    Position lastMove;                    // 最后一步落子位置
    int blackCaptures;                    // 黑方提子数
    int whiteCaptures;                    // 白方提子数
    int blackLiberties;                   // 黑方气数
    int whiteLiberties;                   // 白方气数
    struct BoardHistoryNode* prev;        // 前一个节点
    struct BoardHistoryNode* next;        // 后一个节点
} BoardHistory;

// 棋盘结构
typedef struct {
    Stone board[BOARD_SIZE][BOARD_SIZE];  // 当前棋盘状态
    Stone currentPlayer;                  // 当前玩家
    Position lastMove;                    // 最后一步落子位置
    Position koPosition;                  // 打劫位置
    bool koActive;                        // 是否存在打劫
    int blackCaptures;                    // 黑方提子数
    int whiteCaptures;                    // 白方提子数
    int blackLiberties;                   // 黑方气数
    int whiteLiberties;                   // 白方气数
    BoardHistory* history;                // 历史记录头节点
    BoardHistory* current;                // 当前历史记录节点
} Board;

/**
 * @brief 初始化棋盘
 * @param board 棋盘指针
 */
void initBoard(Board* board);

/**
 * @brief 释放棋盘资源
 * @param board 棋盘指针
 */
void freeBoard(Board* board);

/**
 * @brief 在指定位置落子
 * @param board 棋盘指针
 * @param pos 落子位置
 * @return 落子是否成功
 */
bool placeStone(Board* board, Position pos);

/**
 * @brief 检查指定位置是否有气
 * @param board 棋盘指针
 * @param pos 位置
 * @return 是否有气
 */
bool hasLiberty(Board* board, Position pos);

/**
 * @brief 计算棋子组的气数
 * @param board 棋盘指针
 * @param pos 起始位置
 * @return 气数
 */
int countLiberties(Board* board, Position pos);

/**
 * @brief 提取无气的棋子
 * @param board 棋盘指针
 * @param color 要检查的棋子颜色
 * @return 提取的棋子数量
 */
int captureDeadStones(Board* board, Stone color);

/**
 * @brief 检查落子是否合法
 * @param board 棋盘指针
 * @param pos 落子位置
 * @return 是否合法
 */
bool isValidMove(Board* board, Position pos);

/**
 * @brief 检查是否是自杀行为
 * @param board 棋盘指针
 * @param pos 落子位置
 * @return 是否是自杀行为
 */
bool isSuicideMove(Board* board, Position pos);

/**
 * @brief 检查是否是打劫
 * @param board 棋盘指针
 * @param pos 落子位置
 * @return 是否是打劫
 */
bool isKoMove(Board* board, Position pos);

/**
 * @brief 悔棋
 * @param board 棋盘指针
 * @return 是否成功
 */
bool undoMove(Board* board);

/**
 * @brief 前进一步（取消悔棋）
 * @param board 棋盘指针
 * @return 是否成功
 */
bool redoMove(Board* board);

/**
 * @brief 计算黑白双方的气数
 * @param board 棋盘指针
 */
void calculateLiberties(Board* board);

/**
 * @brief 判断胜负
 * @param board 棋盘指针
 * @return 1表示黑胜，2表示白胜，0表示未分出胜负
 */
int determineWinner(Board* board);

/**
 * @brief 保存当前棋盘状态到历史记录
 * @param board 棋盘指针
 */
void saveBoardState(Board* board);

#endif // BOARD_H