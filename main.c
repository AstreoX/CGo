/**
 * @file main.c
 * @brief 围棋游戏主程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "include/utils.h"
#include "include/board.h"
#include "include/game.h"
#include "include/gui.h"
#include "include/ai.h"

/**
 * @brief 程序入口函数
 */
int main(int argc, char* argv[]) {
    // 初始化随机数生成器
    initRandom();
    
    // 创建资源目录
    if (!fileExists(RESOURCES_DIR)) {
        if (!createDirectory(RESOURCES_DIR)) {
            LOG_ERROR("无法创建资源目录");
            return EXIT_FAILURE;
        }
    }
    
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("SDL初始化失败");
        printf("SDL错误: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    // 初始化SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        LOG_ERROR("SDL_image初始化失败");
        printf("SDL_image错误: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    // 初始化游戏和GUI
    Game game;
    GUI gui;
    
    initGame(&game);
    
    if (!initGUI(&gui)) {
        LOG_ERROR("GUI初始化失败");
        freeGame(&game);
        IMG_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    // 游戏主循环
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        // 处理事件
        running = handleEvents(&gui, &game);
        
        // 更新游戏状态
        updateGame(&game);
        
        // 如果是AI模式且轮到AI下棋且AI未在思考中
        if (game.mode == MODE_PVE && 
            game.state == STATE_PLAYING && 
            game.board.currentPlayer == WHITE && 
            !game.aiThinking) {
            
            game.aiThinking = true;
            handleAIMove(&game);
            game.aiThinking = false;
        }
        
        // 渲染游戏
        renderGame(&gui, &game);
        
        // 帧率控制
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsed = currentTime - lastTime;
        if (elapsed < 16) { // 约60FPS
            SDL_Delay(16 - elapsed);
        }
        lastTime = SDL_GetTicks();
    }
    
    // 清理资源
    freeGUI(&gui);
    freeGame(&game);
    IMG_Quit();
    SDL_Quit();
    
    return EXIT_SUCCESS;
}