#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "go_game.h"
#include "go_ui.h"

// 队列结构（用于广度优先搜索）
typedef struct {
    Position* items;
    int front;
    int rear;
    int capacity;
} Queue;

// 初始化队列
Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->items = (Position*)malloc(capacity * sizeof(Position));
    queue->front = 0;
    queue->rear = -1;
    queue->capacity = capacity;
    return queue;
}

// 入队
void enqueue(Queue* queue, Position pos) {
    if (queue->rear == queue->capacity - 1) return;
    queue->items[++queue->rear] = pos;
}

// 出队
Position dequeue(Queue* queue) {
    return queue->items[queue->front++];
}

// 队列是否为空
bool isQueueEmpty(Queue* queue) {
    return queue->front > queue->rear;
}

// 释放队列
void freeQueue(Queue* queue) {
    free(queue->items);
    free(queue);
}

// 游戏初始化
GameState* initGame() {
    GameState* game = (GameState*)malloc(sizeof(GameState));
    
    // 初始化棋盘
    memset(game->board, EMPTY, sizeof(game->board));
    
    // 初始化游戏状态
    game->currentPlayer = BLACK; // 黑方先行
    game->moveHistory = NULL;
    game->currentMove = NULL;
    game->isGameOver = false;
    game->blackCaptures = 0;
    game->whiteCaptures = 0;
    game->lastCapture.x = -1;
    game->lastCapture.y = -1;
    
    return game;
}

// 清理游戏资源
void cleanupGame(GameState* game) {
    // 清理落子历史链表
    MoveNode* current = game->moveHistory;
    while (current != NULL) {
        MoveNode* next = current->next;
        free(current);
        current = next;
    }
    
    // 释放游戏状态
    free(game);
}

// 添加落子记录
void addMoveToHistory(GameState* game, int x, int y) {
    MoveNode* newNode = (MoveNode*)malloc(sizeof(MoveNode));
    newNode->pos.x = x;
    newNode->pos.y = y;
    newNode->color = game->currentPlayer;
    newNode->next = NULL;
    
    if (game->moveHistory == NULL) {
        // 第一步棋
        newNode->prev = NULL;
        game->moveHistory = newNode;
        game->currentMove = newNode;
    } else {
        // 添加到链表尾部
        newNode->prev = game->currentMove;
        game->currentMove->next = newNode;
        game->currentMove = newNode;
    }
}

// 检查位置是否在棋盘内
bool isOnBoard(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

// 检查劫争
bool isKo(GameState* game, int x, int y) {
    // 如果最后一次提子只提了一个子，并且当前要下的位置正是那个位置，则为劫争
    return (game->lastCapture.x == x && game->lastCapture.y == y);
}

// 计算棋子气数（使用广度优先搜索）
int countLiberties(GameState* game, int x, int y) {
    if (!isOnBoard(x, y) || game->board[y][x] == EMPTY) {
        return 0;
    }
    
    StoneColor color = game->board[y][x];
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    int liberties = 0;
    
    // 方向数组：上、右、下、左
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    Queue* queue = createQueue(BOARD_SIZE * BOARD_SIZE);
    
    // 将起始位置加入队列
    Position start = {x, y};
    enqueue(queue, start);
    visited[y][x] = true;
    
    while (!isQueueEmpty(queue)) {
        Position current = dequeue(queue);
        
        // 检查四个方向
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            
            if (isOnBoard(nx, ny)) {
                if (!visited[ny][nx]) {
                    if (game->board[ny][nx] == EMPTY) {
                        // 找到一口气
                        liberties++;
                        visited[ny][nx] = true;
                    } else if (game->board[ny][nx] == color) {
                        // 相同颜色的棋子，加入队列继续搜索
                        Position next = {nx, ny};
                        enqueue(queue, next);
                        visited[ny][nx] = true;
                    }
                }
            }
        }
    }
    
    freeQueue(queue);
    return liberties;
}

// 提子
int captureStones(GameState* game, int x, int y) {
    if (!isOnBoard(x, y) || game->board[y][x] == EMPTY) {
        return 0;
    }
    
    StoneColor targetColor = game->board[y][x];
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    int captureCount = 0;
    
    // 方向数组：上、右、下、左
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    Queue* queue = createQueue(BOARD_SIZE * BOARD_SIZE);
    Queue* groupQueue = createQueue(BOARD_SIZE * BOARD_SIZE);
    
    // 将起始位置加入队列
    Position start = {x, y};
    enqueue(queue, start);
    enqueue(groupQueue, start);
    visited[y][x] = true;
    
    // 检查这个棋子组是否有气
    bool hasLiberty = false;
    
    while (!isQueueEmpty(queue)) {
        Position current = dequeue(queue);
        
        // 检查四个方向
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            
            if (isOnBoard(nx, ny)) {
                if (game->board[ny][nx] == EMPTY) {
                    // 找到一口气
                    hasLiberty = true;
                } else if (game->board[ny][nx] == targetColor && !visited[ny][nx]) {
                    // 相同颜色的棋子，加入队列继续搜索
                    Position next = {nx, ny};
                    enqueue(queue, next);
                    enqueue(groupQueue, next);
                    visited[ny][nx] = true;
                }
            }
        }
    }
    
    // 如果没有气，提掉这组棋子
    if (!hasLiberty) {
        // 记录最后一次提子的位置（用于劫争判断）
        // 如果只提了一个子，记录其位置
        if (groupQueue->rear == 0) {
            game->lastCapture.x = x;
            game->lastCapture.y = y;
        } else {
            // 提了多个子，不构成劫争
            game->lastCapture.x = -1;
            game->lastCapture.y = -1;
        }
        
        // 提子
        while (!isQueueEmpty(groupQueue)) {
            Position pos = dequeue(groupQueue);
            game->board[pos.y][pos.x] = EMPTY;
            captureCount++;
        }
    } else {
        // 有气，不提子
        game->lastCapture.x = -1;
        game->lastCapture.y = -1;
    }
    
    freeQueue(queue);
    freeQueue(groupQueue);
    
    return captureCount;
}

// 检查落子后是否有气
bool hasLibertyAfterPlacement(GameState* game, int x, int y, StoneColor color) {
    // 临时放置棋子
    StoneColor originalColor = game->board[y][x];
    game->board[y][x] = color;
    
    // 检查是否有气
    int liberties = countLiberties(game, x, y);
    
    // 恢复原状
    game->board[y][x] = originalColor;
    
    return liberties > 0;
}

// 检查落子是否会导致对方棋子被提
bool willCaptureOpponent(GameState* game, int x, int y) {
    StoneColor opponentColor = (game->currentPlayer == BLACK) ? WHITE : BLACK;
    
    // 方向数组：上、右、下、左
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    // 检查四个方向的对方棋子
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if (isOnBoard(nx, ny) && game->board[ny][nx] == opponentColor) {
            // 临时放置棋子
            game->board[y][x] = game->currentPlayer;
            
            // 检查对方棋子是否没气了
            int liberties = countLiberties(game, nx, ny);
            
            // 恢复原状
            game->board[y][x] = EMPTY;
            
            if (liberties == 0) {
                return true;
            }
        }
    }
    
    return false;
}

// 检查落子是否合法
bool isValidMove(GameState* game, int x, int y) {
    // 检查位置是否在棋盘内
    if (!isOnBoard(x, y)) {
        return false;
    }
    
    // 检查位置是否已有棋子
    if (game->board[y][x] != EMPTY) {
        return false;
    }
    
    // 检查劫争
    if (isKo(game, x, y)) {
        return false;
    }
    
    // 检查自杀规则（落子后是否有气）
    if (!hasLibertyAfterPlacement(game, x, y, game->currentPlayer) && 
        !willCaptureOpponent(game, x, y)) {
        return false;
    }
    
    return true;
}

// 落子
bool placeStone(GameState* game, int x, int y) {
    // 检查落子是否合法
    if (!isValidMove(game, x, y)) {
        return false;
    }
    
    // 放置棋子
    game->board[y][x] = game->currentPlayer;
    
    // 添加落子记录
    addMoveToHistory(game, x, y);
    
    // 检查并提取对方的棋子
    int captureCount = 0;
    StoneColor opponentColor = (game->currentPlayer == BLACK) ? WHITE : BLACK;
    
    // 方向数组：上、右、下、左
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if (isOnBoard(nx, ny) && game->board[ny][nx] == opponentColor) {
            captureCount += captureStones(game, nx, ny);
        }
    }
    
    // 更新提子数
    if (game->currentPlayer == BLACK) {
        game->blackCaptures += captureCount;
    } else {
        game->whiteCaptures += captureCount;
    }
    
    // 切换玩家
    switchPlayer(game);
    
    return true;
}

// 切换玩家
void switchPlayer(GameState* game) {
    game->currentPlayer = (game->currentPlayer == BLACK) ? WHITE : BLACK;
}

// 悔棋
bool undoMove(GameState* game) {
    if (game->currentMove == NULL) {
        // 没有落子记录
        return false;
    }
    
    // 获取最后一步棋的信息
    int x = game->currentMove->pos.x;
    int y = game->currentMove->pos.y;
    
    // 清除棋子
    game->board[y][x] = EMPTY;
    
    // 更新当前落子记录
    game->currentMove = game->currentMove->prev;
    if (game->currentMove == NULL) {
        // 回到游戏开始
        game->moveHistory = NULL;
    }
    
    // 切换玩家
    switchPlayer(game);
    
    // 重置劫争标记
    game->lastCapture.x = -1;
    game->lastCapture.y = -1;
    
    return true;
}

// 重做
bool redoMove(GameState* game) {
    if (game->currentMove == NULL) {
        // 没有落子记录或已经是最新的一步
        if (game->moveHistory == NULL) {
            return false;
        }
        
        // 重做第一步
        game->currentMove = game->moveHistory;
    } else if (game->currentMove->next == NULL) {
        // 已经是最新的一步
        return false;
    } else {
        // 重做下一步
        game->currentMove = game->currentMove->next;
    }
    
    // 放置棋子
    int x = game->currentMove->pos.x;
    int y = game->currentMove->pos.y;
    game->board[y][x] = game->currentMove->color;
    
    // 切换玩家
    switchPlayer(game);
    
    return true;
}

// 棋局回溯（复盘）
void replayGame(GameState* game) {
    // 清空棋盘
    memset(game->board, EMPTY, sizeof(game->board));
    
    // 从头开始回放
    MoveNode* current = game->moveHistory;
    game->currentPlayer = BLACK; // 重置为黑方先行
    
    while (current != NULL) {
        // 放置棋子
        placeStone(game, current->pos.x, current->pos.y);
        
        // 暂停一下，让用户观察
        // 在实际实现中，可以添加延时或等待用户输入
        
        current = current->next;
    }
}

// 游戏主循环
void runGameLoop(GameState* game) {
    // 游戏主循环
    // isRunning在go_ui.c中定义，在go_ui.h中声明为extern
    while (isRunning) {
        // 调用UI模块处理用户输入和界面更新
        updateGameUI(game);
    }
}

// AI落子（简单实现，随机选择合法位置）
Position aiMakeMove(GameState* game) {
    Position move = {-1, -1};
    
    // 简单的随机AI
    for (int attempts = 0; attempts < 100; attempts++) {
        int x = rand() % BOARD_SIZE;
        int y = rand() % BOARD_SIZE;
        
        if (isValidMove(game, x, y)) {
            move.x = x;
            move.y = y;
            break;
        }
    }
    
    return move;
}