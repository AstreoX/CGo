/**
 * @file board.c
 * @brief 围棋棋盘数据结构和基本操作实现
 */

#include "../include/board.h"
#include <string.h>

// 方向数组，用于检查相邻位置
static const int DX[4] = {-1, 0, 1, 0};
static const int DY[4] = {0, -1, 0, 1};

/**
 * @brief 检查位置是否在棋盘范围内
 */
static bool isValidPosition(Position pos) {
    return pos.x >= 0 && pos.x < BOARD_SIZE && pos.y >= 0 && pos.y < BOARD_SIZE;
}

/**
 * @brief 创建新的历史记录节点
 */
static BoardHistory* createHistoryNode(Board* board) {
    BoardHistory* node = (BoardHistory*)malloc(sizeof(BoardHistory));
    if (!node) return NULL;
    
    // 复制当前棋盘状态
    memcpy(node->board, board->board, sizeof(board->board));
    node->lastMove = board->lastMove;
    node->blackCaptures = board->blackCaptures;
    node->whiteCaptures = board->whiteCaptures;
    node->blackLiberties = board->blackLiberties;
    node->whiteLiberties = board->whiteLiberties;
    node->prev = NULL;
    node->next = NULL;
    
    return node;
}

void initBoard(Board* board) {
    // 初始化棋盘为空
    memset(board->board, EMPTY, sizeof(board->board));
    
    // 设置初始玩家为黑方
    board->currentPlayer = BLACK;
    
    // 初始化其他属性
    board->lastMove.x = -1;
    board->lastMove.y = -1;
    board->koPosition.x = -1;
    board->koPosition.y = -1;
    board->koActive = false;
    board->blackCaptures = 0;
    board->whiteCaptures = 0;
    board->blackLiberties = 0;
    board->whiteLiberties = 0;
    
    // 创建历史记录头节点
    board->history = createHistoryNode(board);
    board->current = board->history;
}

void freeBoard(Board* board) {
    // 释放历史记录链表
    BoardHistory* current = board->history;
    while (current) {
        BoardHistory* next = current->next;
        free(current);
        current = next;
    }
    
    // 重置指针
    board->history = NULL;
    board->current = NULL;
}

/**
 * @brief 深度优先搜索标记连通的棋子组
 * @param board 棋盘
 * @param pos 起始位置
 * @param color 棋子颜色
 * @param visited 访问标记数组
 * @param group 棋子组数组（输出）
 * @param groupSize 棋子组大小（输出）
 */
static void dfsMarkGroup(Board* board, Position pos, Stone color, bool visited[BOARD_SIZE][BOARD_SIZE], 
                         Position group[], int* groupSize) {
    // 标记当前位置为已访问
    visited[pos.y][pos.x] = true;
    
    // 将当前位置加入棋子组
    group[*groupSize] = pos;
    (*groupSize)++;
    
    // 检查四个相邻位置
    for (int i = 0; i < 4; i++) {
        Position next = {pos.x + DX[i], pos.y + DY[i]};
        
        // 检查位置是否有效且未访问过
        if (isValidPosition(next) && !visited[next.y][next.x] && 
            board->board[next.y][next.x] == color) {
            dfsMarkGroup(board, next, color, visited, group, groupSize);
        }
    }
}

/**
 * @brief 计算棋子组的气数
 * @param board 棋盘
 * @param group 棋子组
 * @param groupSize 棋子组大小
 * @return 气数
 */
static int calculateGroupLiberties(Board* board, Position group[], int groupSize) {
    bool libertyVisited[BOARD_SIZE][BOARD_SIZE] = {false};
    int liberties = 0;
    
    // 检查每个棋子的相邻位置
    for (int i = 0; i < groupSize; i++) {
        Position pos = group[i];
        
        // 检查四个相邻位置
        for (int j = 0; j < 4; j++) {
            Position next = {pos.x + DX[j], pos.y + DY[j]};
            
            // 如果相邻位置是空的且未计算过，则为一口气
            if (isValidPosition(next) && board->board[next.y][next.x] == EMPTY && 
                !libertyVisited[next.y][next.x]) {
                libertyVisited[next.y][next.x] = true;
                liberties++;
            }
        }
    }
    
    return liberties;
}

bool hasLiberty(Board* board, Position pos) {
    if (!isValidPosition(pos)) return false;
    
    Stone color = board->board[pos.y][pos.x];
    if (color == EMPTY) return true;
    
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    Position group[BOARD_SIZE * BOARD_SIZE];
    int groupSize = 0;
    
    // 标记连通的棋子组
    dfsMarkGroup(board, pos, color, visited, group, &groupSize);
    
    // 计算气数
    return calculateGroupLiberties(board, group, groupSize) > 0;
}

int countLiberties(Board* board, Position pos) {
    if (!isValidPosition(pos)) return 0;
    
    Stone color = board->board[pos.y][pos.x];
    if (color == EMPTY) return 0;
    
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    Position group[BOARD_SIZE * BOARD_SIZE];
    int groupSize = 0;
    
    // 标记连通的棋子组
    dfsMarkGroup(board, pos, color, visited, group, &groupSize);
    
    // 计算气数
    return calculateGroupLiberties(board, group, groupSize);
}

int captureDeadStones(Board* board, Stone color) {
    int capturedCount = 0;
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    
    // 遍历整个棋盘
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            // 如果是指定颜色的棋子且未访问过
            if (board->board[y][x] == color && !visited[y][x]) {
                Position pos = {x, y};
                Position group[BOARD_SIZE * BOARD_SIZE];
                int groupSize = 0;
                
                // 标记连通的棋子组
                dfsMarkGroup(board, pos, color, visited, group, &groupSize);
                
                // 如果没有气，则提子
                if (calculateGroupLiberties(board, group, groupSize) == 0) {
                    // 记录最后一个被提的子（用于打劫判断）
                    if (groupSize == 1) {
                        board->koPosition = group[0];
                    } else {
                        board->koPosition.x = -1;
                        board->koPosition.y = -1;
                    }
                    
                    // 提子
                    for (int i = 0; i < groupSize; i++) {
                        board->board[group[i].y][group[i].x] = EMPTY;
                        capturedCount++;
                    }
                }
            }
        }
    }
    
    // 更新提子数
    if (color == BLACK) {
        board->whiteCaptures += capturedCount;
    } else if (color == WHITE) {
        board->blackCaptures += capturedCount;
    }
    
    return capturedCount;
}

bool isKoMove(Board* board, Position pos) {
    // 如果打劫标志未激活，则不是打劫
    if (!board->koActive) return false;
    
    // 检查是否是打劫位置
    return pos.x == board->koPosition.x && pos.y == board->koPosition.y;
}

bool isSuicideMove(Board* board, Position pos) {
    if (!isValidPosition(pos) || board->board[pos.y][pos.x] != EMPTY) {
        return false;
    }
    
    // 临时放置棋子
    Stone originalColor = board->board[pos.y][pos.x];
    board->board[pos.y][pos.x] = board->currentPlayer;
    
    // 检查是否有气
    bool hasLib = hasLiberty(board, pos);
    
    // 恢复原状
    board->board[pos.y][pos.x] = originalColor;
    
    // 如果没有气，则是自杀行为
    return !hasLib;
}

bool isValidMove(Board* board, Position pos) {
    // 检查位置是否在棋盘范围内
    if (!isValidPosition(pos)) return false;
    
    // 检查位置是否为空
    if (board->board[pos.y][pos.x] != EMPTY) return false;
    
    // 检查是否是打劫
    if (isKoMove(board, pos)) return false;
    
    // 检查是否是自杀行为
    if (isSuicideMove(board, pos)) return false;
    
    return true;
}

bool placeStone(Board* board, Position pos) {
    // 检查落子是否合法
    if (!isValidMove(board, pos)) return false;
    
    // 放置棋子
    board->board[pos.y][pos.x] = board->currentPlayer;
    
    // 记录最后一步落子位置
    board->lastMove = pos;
    
    // 重置打劫标志和位置
    board->koActive = false;
    board->koPosition.x = -1;
    board->koPosition.y = -1;
    
    // 提取对方无气的棋子（可能需要多次检查）
    Stone opponentColor = (board->currentPlayer == BLACK) ? WHITE : BLACK;
    int totalCaptured = 0;
    int capturedCount;
    
    do {
        capturedCount = captureDeadStones(board, opponentColor);
        totalCaptured += capturedCount;
    } while (capturedCount > 0);
    
    // 如果提了一个子，可能是打劫
    if (totalCaptured == 1) {
        // 检查落子后是否只有一个棋子且只有一口气
        bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
        Position group[BOARD_SIZE * BOARD_SIZE];
        int groupSize = 0;
        
        dfsMarkGroup(board, pos, board->currentPlayer, visited, group, &groupSize);
        
        if (groupSize == 1 && calculateGroupLiberties(board, group, groupSize) == 1) {
            board->koActive = true;
        }
    }
    
    // 计算双方气数
    calculateLiberties(board);
    
    // 保存当前棋盘状态到历史记录
    saveBoardState(board);
    
    // 切换玩家
    board->currentPlayer = (board->currentPlayer == BLACK) ? WHITE : BLACK;
    
    return true;
}

void calculateLiberties(Board* board) {
    board->blackLiberties = 0;
    board->whiteLiberties = 0;
    
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    
    // 遍历整个棋盘
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            // 如果是棋子且未访问过
            if ((board->board[y][x] == BLACK || board->board[y][x] == WHITE) && !visited[y][x]) {
                Position pos = {x, y};
                Position group[BOARD_SIZE * BOARD_SIZE];
                int groupSize = 0;
                
                // 标记连通的棋子组
                dfsMarkGroup(board, pos, board->board[y][x], visited, group, &groupSize);
                
                // 计算气数
                int liberties = calculateGroupLiberties(board, group, groupSize);
                
                // 更新对应颜色的总气数
                if (board->board[y][x] == BLACK) {
                    board->blackLiberties += liberties;
                } else if (board->board[y][x] == WHITE) {
                    board->whiteLiberties += liberties;
                }
            }
        }
    }
}

int determineWinner(Board* board) {
    // 计算各方占据的交叉点和提子
    int blackPoints = 0;
    int whitePoints = 0;
    
    // 标记数组，用于记录已检查过的空地
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    
    // 第一步：统计双方直接占据的交叉点
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (board->board[y][x] == BLACK) {
                blackPoints++;
            } else if (board->board[y][x] == WHITE) {
                whitePoints++;
            }
        }
    }
    
    // 第二步：计算空地的归属（确定地盘）
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            // 如果是空地且未访问过
            if (board->board[y][x] == EMPTY && !visited[y][x]) {
                // 找出与该空地相连的所有空地
                int territorySize = 0;
                bool touchesBlack = false;
                bool touchesWhite = false;
                
                // DFS查找连通的空地
                Position queue[BOARD_SIZE * BOARD_SIZE];
                int front = 0, rear = 0;
                
                // 添加初始点
                Position startPos = {x, y};
                queue[rear++] = startPos;
                visited[y][x] = true;
                territorySize++;
                
                // BFS遍历
                while (front < rear) {
                    Position current = queue[front++];
                    
                    // 检查四个方向
                    for (int d = 0; d < 4; d++) {
                        int dx[4] = {-1, 0, 1, 0};
                        int dy[4] = {0, -1, 0, 1};
                        
                        int nx = current.x + dx[d];
                        int ny = current.y + dy[d];
                        
                        // 检查边界
                        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) {
                            continue;
                        }
                        
                        // 如果是空地且未访问过
                        if (board->board[ny][nx] == EMPTY && !visited[ny][nx]) {
                            Position nextPos = {nx, ny};
                            queue[rear++] = nextPos;
                            visited[ny][nx] = true;
                            territorySize++;
                        }
                        // 检查是否接触到黑子或白子
                        else if (board->board[ny][nx] == BLACK) {
                            touchesBlack = true;
                        }
                        else if (board->board[ny][nx] == WHITE) {
                            touchesWhite = true;
                        }
                    }
                }
                
                // 判断空地归属
                if (touchesBlack && !touchesWhite) {
                    // 空地只与黑子相邻，归黑方
                    blackPoints += territorySize;
                } else if (!touchesBlack && touchesWhite) {
                    // 空地只与白子相邻，归白方
                    whitePoints += territorySize;
                }
                // 如果与双方都相邻或都不相邻，则为中立点，不计分
            }
        }
    }
    
    // 第三步：加上提子数
    blackPoints += board->whiteCaptures;
    whitePoints += board->blackCaptures;
    
    // 第四步：加上贴目（白棋有3.75目的贴目优势，简化为4目）
    whitePoints += 4;  // 贴目（中国规则通常为3.75，简化为4）
    
    // 确定胜者
    if (blackPoints > whitePoints) {
        return BLACK;  // 黑胜
    } else if (whitePoints > blackPoints) {
        return WHITE;  // 白胜
    } else {
        return 0;      // 平局（实际围棋很少出现平局）
    }
}

void saveBoardState(Board* board) {
    // 创建新的历史记录节点
    BoardHistory* newNode = createHistoryNode(board);
    if (!newNode) return;
    
    // 如果当前节点不是链表尾，则删除后续节点
    BoardHistory* next = board->current->next;
    while (next) {
        BoardHistory* temp = next;
        next = next->next;
        free(temp);
    }
    
    // 连接新节点
    board->current->next = newNode;
    newNode->prev = board->current;
    newNode->next = NULL;
    
    // 更新当前节点
    board->current = newNode;
}

bool undoMove(Board* board) {
    // 检查是否可以悔棋
    if (!board->current || !board->current->prev) {
        return false;
    }
    
    // 移动到前一个历史记录节点
    board->current = board->current->prev;
    
    // 恢复棋盘状态
    memcpy(board->board, board->current->board, sizeof(board->board));
    board->lastMove = board->current->lastMove;
    board->blackCaptures = board->current->blackCaptures;
    board->whiteCaptures = board->current->whiteCaptures;
    board->blackLiberties = board->current->blackLiberties;
    board->whiteLiberties = board->current->whiteLiberties;
    
    // 切换玩家
    board->currentPlayer = (board->currentPlayer == BLACK) ? WHITE : BLACK;
    
    // 重置打劫标志
    board->koActive = false;
    board->koPosition.x = -1;
    board->koPosition.y = -1;
    
    return true;
}

bool redoMove(Board* board) {
    // 检查是否可以取消悔棋
    if (!board->current || !board->current->next) {
        return false;
    }
    
    // 移动到后一个历史记录节点
    board->current = board->current->next;
    
    // 恢复棋盘状态
    memcpy(board->board, board->current->board, sizeof(board->board));
    board->lastMove = board->current->lastMove;
    board->blackCaptures = board->current->blackCaptures;
    board->whiteCaptures = board->current->whiteCaptures;
    board->blackLiberties = board->current->blackLiberties;
    board->whiteLiberties = board->current->whiteLiberties;
    
    // 切换玩家
    board->currentPlayer = (board->currentPlayer == BLACK) ? WHITE : BLACK;
    
    // 重置打劫标志
    board->koActive = false;
    board->koPosition.x = -1;
    board->koPosition.y = -1;
    
    return true;
}