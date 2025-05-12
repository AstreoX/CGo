/**
 * @file ai.h
 * @brief 围棋AI算法（蒙特卡洛树搜索）
 * 
 * 优化点:
 * 1. 时间限制：为MCTS设置了时间限制而不是固定迭代次数，以确保响应性
 * 2. 范围限制：在对手上次落子附近的小范围内搜索，减少搜索空间
 * 3. 修改UCT公式：随着访问次数增加动态调整探索权重
 * 4. 前几步特殊处理：首步采用天元或星位等策略性位置
 * 5. 减少模拟深度：不模拟到游戏结束，而是运行有限步数
 * 6. 局部搜索：优先在有意义的位置搜索，如已有棋子附近
 * 7. 提前终止：如果多步无提子，提前结束模拟
 * 8. 渐进式扩展：增加随机探索的概率以避免局部最优
 */

#ifndef AI_H
#define AI_H

#include "board.h"

// 前向声明
typedef struct Game Game;

// 蒙特卡洛树节点
typedef struct MCTSNode {
    Position move;                // 此节点对应的落子位置
    Stone player;                 // 此节点对应的玩家
    int visits;                   // 访问次数
    double wins;                  // 胜利次数
    int childrenCount;            // 子节点数量
    struct MCTSNode** children;   // 子节点数组
    struct MCTSNode* parent;      // 父节点
} MCTSNode;

// AI配置
typedef struct {
    int simulationCount;          // 每步模拟次数
    double explorationParameter;  // UCT探索参数
    int maxDepth;                 // 最大搜索深度
} AIConfig;

/**
 * @brief 初始化AI配置
 * @param config AI配置指针
 */
void initAIConfig(AIConfig* config);

/**
 * @brief 创建MCTS根节点
 * @param board 当前棋盘状态
 * @return 创建的根节点
 */
MCTSNode* createRootNode(Board* board);

/**
 * @brief 释放MCTS树
 * @param node 要释放的节点
 */
void freeMCTSTree(MCTSNode* node);

/**
 * @brief 使用蒙特卡洛树搜索选择最佳落子位置
 * @param board 当前棋盘状态
 * @param config AI配置
 * @return 最佳落子位置
 */
Position findBestMove(Board* board, AIConfig* config);

/**
 * @brief 执行一次蒙特卡洛树搜索
 * @param board 当前棋盘状态
 * @param config AI配置
 * @param root 根节点
 */
void runMCTS(Board* board, AIConfig* config, MCTSNode* root);

/**
 * @brief 选择阶段 - 选择最有前途的节点
 * @param node 当前节点
 * @param config AI配置
 * @return 选择的节点
 */
MCTSNode* selectNode(MCTSNode* node, AIConfig* config);

/**
 * @brief 扩展阶段 - 扩展选择的节点
 * @param node 要扩展的节点
 * @param board 当前棋盘状态
 * @return 新创建的子节点
 */
MCTSNode* expandNode(MCTSNode* node, Board* board);

/**
 * @brief 模拟阶段 - 从给定节点开始随机模拟到游戏结束
 * @param node 开始模拟的节点
 * @param board 当前棋盘状态
 * @param config AI配置
 * @return 模拟结果（胜利为1，失败为0）
 */
double simulateGame(MCTSNode* node, Board* board, AIConfig* config);

/**
 * @brief 反向传播阶段 - 更新节点统计信息
 * @param node 开始更新的节点
 * @param result 模拟结果
 */
void backpropagate(MCTSNode* node, double result);

/**
 * @brief 根据UCT值选择最佳子节点
 * @param node 父节点
 * @param config AI配置
 * @return 最佳子节点
 */
MCTSNode* selectBestChild(MCTSNode* node, AIConfig* config);

/**
 * @brief 获取所有合法落子位置
 * @param board 当前棋盘状态
 * @param positions 位置数组（输出）
 * @return 合法落子位置数量
 */
int getLegalMoves(Board* board, Position* positions);

#endif // AI_H