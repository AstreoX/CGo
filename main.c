#include <stdio.h>
#include <stdlib.h>
#include "go_game.h"
#include "go_ui.h"

int main() {
    // 初始化游戏
    GameState* game = initGame();
    
    // 初始化图形界面
    initUI(game);
    
    // 游戏主循环
    runGameLoop(game);
    
    // 清理资源
    cleanupGame(game);
    cleanupUI();
    
    return 0;
}