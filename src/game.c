/**
 * @file game.c
 * @brief 围棋游戏逻辑和规则实现
 */

#include "../include/game.h"
#include "../include/ai.h"
#include <string.h>

void initGame(Game* game) {
    // 初始化棋盘
    initBoard(&game->board);
    
    // 设置初始游戏模式和状态
    game->mode = MODE_PVP;
    game->state = STATE_PLAYING;
    game->showHints = true;
    game->aiThinking = false;
    game->winner = 0;
}

void freeGame(Game* game) {
    // 释放棋盘资源
    freeBoard(&game->board);
}

bool handlePlayerMove(Game* game, Position pos) {
    // 如果游戏未在进行中，则不处理落子
    if (game->state != STATE_PLAYING) {
        return false;
    }
    
    // 如果是人机模式且轮到AI下棋，则不处理玩家落子
    if (game->mode == MODE_PVE && game->board.currentPlayer == WHITE) {
        return false;
    }
    
    // 尝试落子
    bool success = placeStone(&game->board, pos);
    
    // 如果落子成功，检查游戏是否结束
    if (success) {
        updateGame(game);
    }
    
    return success;
}

bool handleAIMove(Game* game) {
    // 如果游戏未在进行中或不是AI的回合，则不处理
    if (game->state != STATE_PLAYING || 
        game->mode != MODE_PVE || 
        game->board.currentPlayer != WHITE) {
        return false;
    }
    
    // 初始化AI配置
    AIConfig config;
    initAIConfig(&config);
    
    // 使用蒙特卡洛树搜索找到最佳落子位置
    Position bestMove = findBestMove(&game->board, &config);
    
    // 尝试落子
    bool success = placeStone(&game->board, bestMove);
    
    // 如果落子成功，检查游戏是否结束
    if (success) {
        updateGame(game);
    }
    
    return success;
}

void toggleGameMode(Game* game) {
    // 切换游戏模式
    game->mode = (game->mode == MODE_PVP) ? MODE_PVE : MODE_PVP;
}

void toggleHints(Game* game) {
    // 切换提示显示
    game->showHints = !game->showHints;
}

bool handleUndo(Game* game) {
    // 如果游戏未在进行中，则不处理悔棋
    if (game->state != STATE_PLAYING) {
        return false;
    }
    
    // 尝试悔棋
    bool success = undoMove(&game->board);
    
    // 如果是人机模式且悔棋后轮到AI，则再悔一步
    if (success && game->mode == MODE_PVE && game->board.currentPlayer == WHITE) {
        success = undoMove(&game->board);
    }
    
    return success;
}

bool handleRedo(Game* game) {
    // 如果游戏未在进行中，则不处理前进
    if (game->state != STATE_PLAYING) {
        return false;
    }
    
    // 尝试前进
    bool success = redoMove(&game->board);
    
    // 如果是人机模式且前进后轮到AI，则再前进一步
    if (success && game->mode == MODE_PVE && game->board.currentPlayer == WHITE) {
        success = redoMove(&game->board);
    }
    
    return success;
}

void updateGame(Game* game) {
    // 计算双方气数
    calculateLiberties(&game->board);
    
    // 检查游戏是否结束
    // 游戏结束的条件：
    // 1. 棋盘上没有空位（或空位很少）
    // 2. 玩家双方都同意结束游戏（在GUI中通过按下E键实现）
    
    // 检查棋盘上的空位数量
    int emptyCount = 0;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (game->board.board[y][x] == EMPTY) {
                emptyCount++;
                
                // 如果空位太多，游戏还未结束
                if (emptyCount > 10) {
                    return;
                }
            }
        }
    }
    
    // 空位很少，游戏自动结束
    if (emptyCount <= 10) {
        endGameManually(game);
    }
}

/**
 * @brief 手动结束游戏并计算胜负
 * @param game 游戏指针
 */
void endGameManually(Game* game) {
    // 确保游戏在进行中
    if (game->state != STATE_PLAYING) {
        return;
    }
    
    // 计算双方气数（确保数据最新）
    calculateLiberties(&game->board);
    
    // 设置游戏状态为结束
    game->state = STATE_GAMEOVER;
    
    // 判定胜负
    game->winner = determineWinner(&game->board);
}

bool isGameOver(Game* game) {
    return game->state == STATE_GAMEOVER;
}

const char* getViolationHint(Game* game, Position pos) {
    // 如果不显示提示，则返回NULL
    if (!game->showHints) {
        return NULL;
    }
    
    // 检查位置是否在棋盘范围内
    if (pos.x < 0 || pos.x >= BOARD_SIZE || pos.y < 0 || pos.y >= BOARD_SIZE) {
        return "违规行为：位置超出棋盘范围";
    }
    
    // 检查位置是否已有棋子
    if (game->board.board[pos.y][pos.x] != EMPTY) {
        return "违规行为：该位置已有棋子";
    }
    
    // 检查是否是打劫
    if (isKoMove(&game->board, pos)) {
        return "违规行为：打劫";
    }
    
    // 检查是否是自杀行为
    if (isSuicideMove(&game->board, pos)) {
        return "违规行为：自杀";
    }
    
    return NULL;
}