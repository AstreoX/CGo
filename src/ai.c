/**
 * @file ai.c
 * @brief 围棋AI算法（蒙特卡洛树搜索）实现
 */

#include "../include/ai.h"
#include "../include/utils.h"
#include "../include/game.h"
#include <math.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

// 方向数组，用于检查相邻位置
static const int DX[4] = {-1, 0, 1, 0}; // Used in expansion strategies
static const int DY[4] = {0, -1, 0, 1}; // Used in expansion strategies

// 优化的AI配置参数
#define DEFAULT_SIMULATION_COUNT 789  // 增加模拟次数
#define DEFAULT_EXPLORATION_PARAM 4.2 // UCT探索参数
#define DEFAULT_MAX_DEPTH 50         // 减少最大搜索深度
#define MCTS_TIME_LIMIT 3000         // 时间限制
#define MCTS_RANGE_SMALL 1           // 小范围搜索3×3

void initAIConfig(AIConfig* config) {
    config->simulationCount = DEFAULT_SIMULATION_COUNT;
    config->explorationParameter = DEFAULT_EXPLORATION_PARAM;
    config->maxDepth = DEFAULT_MAX_DEPTH;
}

MCTSNode* createRootNode(Board* board) {
    MCTSNode* root = (MCTSNode*)malloc(sizeof(MCTSNode));
    if (!root) return NULL;
    
    // 初始化根节点
    root->move.x = -1;
    root->move.y = -1;
    root->player = board->currentPlayer;
    root->visits = 0;
    root->wins = 0;
    root->childrenCount = 0;
    root->children = NULL;
    root->parent = NULL;
    
    return root;
}

void freeMCTSTree(MCTSNode* node) {
    if (!node) return;
    
    // 递归释放所有子节点
    for (int i = 0; i < node->childrenCount; i++) {
        freeMCTSTree(node->children[i]);
    }
    
    // 释放子节点数组
    if (node->children) {
        free(node->children);
    }
    
    // 释放节点本身
    free(node);
}

/**
 * @brief 创建子节点
 * @param parent 父节点
 * @param move 落子位置
 * @param player 玩家
 * @return 创建的子节点
 */
static MCTSNode* createChildNode(MCTSNode* parent, Position move, Stone player) {
    MCTSNode* child = (MCTSNode*)malloc(sizeof(MCTSNode));
    if (!child) return NULL;
    
    // 初始化子节点
    child->move = move;
    child->player = player;
    child->visits = 0;
    child->wins = 0;
    child->childrenCount = 0;
    child->children = NULL;
    child->parent = parent;
    
    return child;
}

/**
 * @brief 计算UCT值
 * @param node 节点
 * @param parentVisits 父节点访问次数
 * @param explorationParam 探索参数
 * @return UCT值
 */
static double calculateUCT(MCTSNode* node, int parentVisits, double explorationParam) {
    if (node->visits == 0) {
        return INFINITY; // 未访问过的节点优先选择
    }
    
    // 优化的UCT公式，随着访问次数逐渐减小探索权重
    double exploitation = node->wins / node->visits;
    double visitFactor = sqrt(2.0 - (node->visits / (double)(parentVisits + 1)));
    double exploration = explorationParam * visitFactor * sqrt(log(parentVisits) / node->visits);
    
    return exploitation + exploration;
}

/**
 * @brief 获取特定范围内的有效移动
 * @param board 棋盘
 * @param centerX 中心X坐标
 * @param centerY 中心Y坐标
 * @param range 范围
 * @param positions 输出位置数组
 * @param count 计数指针
 */
static void getValidMovesInRange(Board* board, int centerX, int centerY, int range, Position* positions, int* count) {
    *count = 0;
    
    for (int dx = -range; dx <= range; dx++) {
        for (int dy = -range; dy <= range; dy++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            Position pos = {x, y};
            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && isValidMove(board, pos)) {
                positions[*count] = pos;
                (*count)++;
            }
        }
    }
}

MCTSNode* selectNode(MCTSNode* node, AIConfig* config) {
    // 如果节点没有子节点，则返回该节点
    if (node->childrenCount == 0) {
        return node;
    }
    
    // 渐进式扩展 - 随机选择的概率随访问次数增加而减小
    if (rand() % 100 < 5 && node->visits > 50) { // 5%的概率随机选择，且节点被访问过至少50次
        int randomChild = rand() % node->childrenCount;
        return selectNode(node->children[randomChild], config);
    }
    
    // 选择UCT值最大的子节点
    MCTSNode* bestChild = NULL;
    double bestUCT = -INFINITY;
    
    for (int i = 0; i < node->childrenCount; i++) {
        double uct = calculateUCT(node->children[i], node->visits, config->explorationParameter);
        
        if (uct > bestUCT) {
            bestUCT = uct;
            bestChild = node->children[i];
        }
    }
    
    // 递归选择
    return selectNode(bestChild, config);
}

int getLegalMoves(Board* board, Position* positions) {
    int count = 0;
    
    // 遍历整个棋盘，找出所有合法落子位置
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Position pos = {x, y};
            if (isValidMove(board, pos)) {
                positions[count++] = pos;
            }
        }
    }
    
    return count;
}

MCTSNode* expandNode(MCTSNode* node, Board* board) {
    // 创建临时棋盘用于模拟
    Board tempBoard;
    memcpy(&tempBoard, board, sizeof(Board));
    
    // 模拟到当前节点的状态
    MCTSNode* current = node;
    while (current->parent) {
        // 回溯到根节点，记录每一步
        MCTSNode* moves[DEFAULT_MAX_DEPTH];
        int moveCount = 0;
        
        MCTSNode* temp = current;
        while (temp->parent) {
            moves[moveCount++] = temp;
            temp = temp->parent;
        }
        
        // 按照正确顺序应用这些移动
        for (int i = moveCount - 1; i >= 0; i--) {
            placeStone(&tempBoard, moves[i]->move);
        }
        
        current = current->parent;
    }
    
    // 获取所有合法落子位置 - 优化：考虑距离上次落子的范围
    Position legalMoves[BOARD_SIZE * BOARD_SIZE];
    int legalMoveCount = 0;
    
    // 如果可以，在上一步落子的周围5×5范围内搜索
    if (tempBoard.lastMove.x >= 0 && tempBoard.lastMove.y >= 0) {
        getValidMovesInRange(&tempBoard, tempBoard.lastMove.x, tempBoard.lastMove.y, MCTS_RANGE_SMALL, legalMoves, &legalMoveCount);
    }
    
    // 如果在小范围内没有找到足够的落子点，考虑一些战略位置
    if (legalMoveCount < 5) {
        // 考虑天元和星位
        int starPoints[][2] = {
            {3, 3}, {3, BOARD_SIZE-4}, {BOARD_SIZE-4, 3}, {BOARD_SIZE-4, BOARD_SIZE-4},
            {3, BOARD_SIZE/2}, {BOARD_SIZE/2, 3}, {BOARD_SIZE-4, BOARD_SIZE/2}, {BOARD_SIZE/2, BOARD_SIZE-4},
            {BOARD_SIZE/2, BOARD_SIZE/2} // 天元
        };
        
        for (int i = 0; i < 9; i++) {
            Position pos = {starPoints[i][0], starPoints[i][1]};
            if (isValidMove(&tempBoard, pos)) {
                // 检查是否已经在列表中
                bool exists = false;
                for (int j = 0; j < legalMoveCount; j++) {
                    if (legalMoves[j].x == pos.x && legalMoves[j].y == pos.y) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    legalMoves[legalMoveCount++] = pos;
                }
            }
        }
    }
    
    // 如果仍然没有足够的合法落子，获取所有合法落子
    if (legalMoveCount < 5) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                // 跳过已经在列表中的位置
                bool exists = false;
                for (int j = 0; j < legalMoveCount; j++) {
                    if (legalMoves[j].x == x && legalMoves[j].y == y) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    Position pos = {x, y};
                    if (isValidMove(&tempBoard, pos)) {
                        legalMoves[legalMoveCount++] = pos;
                        
                        // 如果已经找到足够多的落子点，就停止搜索
                        if (legalMoveCount >= 20) {
                            break;
                        }
                    }
                }
            }
            if (legalMoveCount >= 20) break;
        }
    }
    
    // 如果没有合法落子，则返回当前节点
    if (legalMoveCount == 0) {
        return node;
    }
    
    // 为每个合法落子创建子节点
    node->childrenCount = legalMoveCount;
    node->children = (MCTSNode**)malloc(sizeof(MCTSNode*) * legalMoveCount);
    
    if (!node->children) {
        node->childrenCount = 0;
        return node;
    }
    
    // 计算下一个玩家
    Stone nextPlayer = (node->player == BLACK) ? WHITE : BLACK;
    
    // 创建子节点
    for (int i = 0; i < legalMoveCount; i++) {
        node->children[i] = createChildNode(node, legalMoves[i], nextPlayer);
    }
    
    // 改进：使用三种策略之一选择子节点
    int strategy = rand() % 3;
    int selectedIndex = 0;
    
    if (strategy == 0) { // 随机选择
        selectedIndex = rand() % legalMoveCount;
    }
    else if (strategy == 1 && tempBoard.lastMove.x >= 0) { // 边缘策略
        // 找到距离上一个落子点最远的点
        int parentX = tempBoard.lastMove.x;
        int parentY = tempBoard.lastMove.y;
        int maxDistance = 0;
        
        for (int i = 0; i < legalMoveCount; i++) {
            int dx = legalMoves[i].x - parentX;
            int dy = legalMoves[i].y - parentY;
            int distance = dx*dx + dy*dy;
            
            if (distance > maxDistance) {
                maxDistance = distance;
                selectedIndex = i;
            }
        }
    }
    else { // 中心策略
        // 找到距离棋盘中心最近的点
        int centerX = BOARD_SIZE / 2;
        int centerY = BOARD_SIZE / 2;
        int minDistance = BOARD_SIZE * BOARD_SIZE * 2;
        
        for (int i = 0; i < legalMoveCount; i++) {
            int dx = legalMoves[i].x - centerX;
            int dy = legalMoves[i].y - centerY;
            int distance = dx*dx + dy*dy;
            
            // 额外考虑四个方向是否有自己的棋子，提高连接性
            int connectedStones = 0;
            for (int j = 0; j < 4; j++) {
                int nx = legalMoves[i].x + DX[j];
                int ny = legalMoves[i].y + DY[j];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                    tempBoard.board[ny][nx] == node->player) {
                    connectedStones++;
                }
            }
            
            // 减少距离以优先选择连接性好的位置
            distance -= connectedStones * 5;
            
            if (distance < minDistance) {
                minDistance = distance;
                selectedIndex = i;
            }
        }
    }
    
    return node->children[selectedIndex];
}

double simulateGame(MCTSNode* node, Board* board, AIConfig* config) {
    // 标记参数已使用
    (void)config;
    
    // 创建临时棋盘用于模拟
    Board tempBoard;
    memcpy(&tempBoard, board, sizeof(Board));
    
    // 模拟到当前节点的状态
    MCTSNode* current = node;
    while (current->parent) {
        // 回溯到根节点，记录每一步
        MCTSNode* moves[DEFAULT_MAX_DEPTH];
        int moveCount = 0;
        
        MCTSNode* temp = current;
        while (temp->parent) {
            moves[moveCount++] = temp;
            temp = temp->parent;
        }
        
        // 按照正确顺序应用这些移动
        for (int i = moveCount - 1; i >= 0; i--) {
            placeStone(&tempBoard, moves[i]->move);
        }
        
        current = current->parent;
    }
    
    // 减少模拟步数，提高速度
    int maxMoves = 40 + (rand() % 20);  // 40-60步
    int moveCount = 0;
    Stone currentPlayer = node->player;
    int prevBlackCaptured = tempBoard.blackCaptures;
    int prevWhiteCaptured = tempBoard.whiteCaptures;
    
    while (moveCount < maxMoves) {
        // 获取所有有效移动 - 只考虑5×5范围内的移动以加快模拟
        Position moves[BOARD_SIZE * BOARD_SIZE];
        int validMoveCount = 0;
        
        if (tempBoard.lastMove.x >= 0 && tempBoard.lastMove.y >= 0) {
            getValidMovesInRange(&tempBoard, tempBoard.lastMove.x, tempBoard.lastMove.y, MCTS_RANGE_SMALL, moves, &validMoveCount);
        }
        
        // 如果找不到有效移动，扩大搜索范围
        if (validMoveCount == 0) {
            for (int attempts = 0; attempts < 10 && validMoveCount == 0; attempts++) {
                int x = rand() % BOARD_SIZE;
                int y = rand() % BOARD_SIZE;
                Position pos = {x, y};
                if (isValidMove(&tempBoard, pos)) {
                    moves[validMoveCount++] = pos;
                }
            }
        }
        
        // 如果没有有效移动，结束模拟
        if (validMoveCount == 0) {
            break;
        }
        
        // 简单随机选择，不使用启发式以提高速度
        int selectedMove = rand() % validMoveCount;
        Position movePos = moves[selectedMove];
        
        // 走子
        placeStone(&tempBoard, movePos);
        
        moveCount++;
        
        // 如果连续5步都没有提子，提前结束模拟
        if (moveCount > 20 && moveCount % 5 == 0) {
            bool shouldEnd = true;
            if (tempBoard.blackCaptures != prevBlackCaptured || 
                tempBoard.whiteCaptures != prevWhiteCaptured) {
                shouldEnd = false;
            }
            
            if (shouldEnd) break;
            
            prevBlackCaptured = tempBoard.blackCaptures;
            prevWhiteCaptured = tempBoard.whiteCaptures;
        }
        
        // 切换玩家
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    }
    
    // 简化评分计算，减少计算开销
    double score;
    
    // 提子加权评分 - 使用黑方和白方的提子数作为评分
    int blackScore = tempBoard.blackCaptures;
    int whiteScore = tempBoard.whiteCaptures;
    
    // 根据当前玩家返回相应评分
    if ((int)node->player == BLACK) {
        score = (blackScore > whiteScore) ? 1.0 : 0.0;
    } else {
        score = (whiteScore > blackScore) ? 1.0 : 0.0;
    }
    
    return score;
}

void backpropagate(MCTSNode* node, double result) {
    MCTSNode* current = node;
    
    while (current) {
        current->visits++;
        
        // 更新胜率（从各自玩家角度）
        if (current->player == node->player) {
            current->wins += result;
        } else {
            current->wins += (1.0 - result);
        }
        
        current = current->parent;
    }
}

MCTSNode* selectBestChild(MCTSNode* node, AIConfig* config) {
    (void)config; // 标记参数已使用
    
    if (!node || node->childrenCount == 0) {
        return NULL;
    }
    
    MCTSNode* bestChild = NULL;
    double bestScore = -INFINITY;
    
    // 选择访问次数最多的子节点（最可靠的选择）
    for (int i = 0; i < node->childrenCount; i++) {
        if (node->children[i]->visits > bestScore) {
            bestScore = node->children[i]->visits;
            bestChild = node->children[i];
        }
    }
    
    return bestChild;
}

void runMCTS(Board* board, AIConfig* config, MCTSNode* root) {
    // 使用时间限制而不是固定迭代次数
    Uint32 startTime = SDL_GetTicks();
    int iterations = 0;
    
    while (iterations < config->simulationCount && (SDL_GetTicks() - startTime) < MCTS_TIME_LIMIT) {
        // 选择阶段
        MCTSNode* selected = selectNode(root, config);
        
        // 扩展阶段
        MCTSNode* expanded = expandNode(selected, board);
        
        // 模拟阶段
        double result = simulateGame(expanded, board, config);
        
        // 反向传播阶段
        backpropagate(expanded, result);
        
        iterations++;
    }
}

Position findBestMove(Board* board, AIConfig* config) {
    // 初始化随机数生成器
    srand((unsigned)time(NULL));
    
    // 创建根节点
    MCTSNode* root = createRootNode(board);
    
    // 获取有效移动 - 优化：第一种情况：如果是游戏开始则考虑天元和星位
    Position validMoves[BOARD_SIZE * BOARD_SIZE];
    int validMoveCount = 0;
    
    // 如果是第一步（没有历史移动），优先选择天元和星位
    if (board->lastMove.x < 0 || board->lastMove.y < 0) {
        // 天元 (棋盘中心)
        int center = BOARD_SIZE / 2;
        Position centerPos = {center, center};
        if (isValidMove(board, centerPos)) {
            validMoves[validMoveCount++] = centerPos;
        }
        
        // 星位点
        int starPoints[][2] = {
            {3, 3}, {3, BOARD_SIZE-4}, {BOARD_SIZE-4, 3}, {BOARD_SIZE-4, BOARD_SIZE-4},
            {3, center}, {center, 3}, {BOARD_SIZE-4, center}, {center, BOARD_SIZE-4}
        };
        
        for (int i = 0; i < 8; i++) {
            Position pos = {starPoints[i][0], starPoints[i][1]};
            if (isValidMove(board, pos)) {
                validMoves[validMoveCount++] = pos;
            }
        }
        
        // 如果有有效的天元或星位，直接随机选择一个
        if (validMoveCount > 0) {
            int randomIndex = rand() % validMoveCount;
            Position bestMove = validMoves[randomIndex];
            
            // 释放树
            freeMCTSTree(root);
        //打印MCTS搜索信息
            printf("MCTS: Found best move at (%d, %d) in first turn.\n", bestMove.x, bestMove.y);
            
            return bestMove;
        }
    }
    
    // 第二种情况：在对手上次落子的5×5范围内搜索
    if (board->lastMove.x >= 0 && board->lastMove.y >= 0) {
        getValidMovesInRange(board, board->lastMove.x, board->lastMove.y, MCTS_RANGE_SMALL, validMoves, &validMoveCount);
        
        // 保存可用的有效落子点，用于超时情况
        Position backupMoves[BOARD_SIZE * BOARD_SIZE];
        int backupCount = validMoveCount;
        memcpy(backupMoves, validMoves, validMoveCount * sizeof(Position));
        
        // 运行MCTS
        runMCTS(board, config, root);
        
        // 选择最佳子节点
        MCTSNode* bestChild = selectBestChild(root, config);
        
        // 获取最佳落子位置
        Position bestMove;
        if (bestChild) {
            bestMove = bestChild->move;
        } else if (backupCount > 0) {
            // 如果MCTS失败，从备份中随机选择
            int randomIndex = rand() % backupCount;
            bestMove = backupMoves[randomIndex];
        } else {
            // 如果没有有效落子，返回无效位置
            bestMove.x = -1;
            bestMove.y = -1;
        }
        
        // 释放树
        freeMCTSTree(root);
        
        return bestMove;
    }
    
    // 如果没有历史移动或可用的有效位置，获取所有合法移动
    validMoveCount = getLegalMoves(board, validMoves);
    
    // 如果没有合法移动，返回无效位置
    if (validMoveCount == 0) {
        Position invalidMove = {-1, -1};
        freeMCTSTree(root);
        return invalidMove;
    }
    
    // 运行MCTS
    runMCTS(board, config, root);
    
    // 选择最佳子节点
    MCTSNode* bestChild = selectBestChild(root, config);
    
    // 获取最佳落子位置
    Position bestMove;
    if (bestChild) {
        bestMove = bestChild->move;
    } else {
        // 如果没有找到最佳落子，随机选择一个合法位置
        int randomIndex = rand() % validMoveCount;
        bestMove = validMoves[randomIndex];
    }
    
    // 释放树
    freeMCTSTree(root);
    //打印MCTS搜索信息
    printf("MCTS: Found best move at (%d, %d) after %d iterations.\n", bestMove.x, bestMove.y, config->simulationCount);
    return bestMove;
}

/**
 * @brief 执行AI落子（适用于当前玩家，无论黑白）
 * @param game 游戏指针
 * @return 落子是否成功
 */
bool makeAIMoveForCurrentPlayer(Game* game) {
    // 如果游戏未在进行中，则不处理
    if (game->state != STATE_PLAYING) {
        return false;
    }
    
    // 标记AI正在思考
    game->aiThinking = true;
    
    // 初始化AI配置
    AIConfig config;
    initAIConfig(&config);
    
    // 使用蒙特卡洛树搜索找到最佳落子位置
    Position bestMove = findBestMove(&game->board, &config);
    
    // 尝试落子
    bool success = placeStone(&game->board, bestMove);
    
    // 如果落子成功，更新游戏状态
    if (success) {
        updateGame(game);
    }
    
    // AI思考结束
    game->aiThinking = false;
    
    return success;
}