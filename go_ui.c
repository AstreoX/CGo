#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "go_ui.h"
#include "go_game.h"

// 窗口尺寸
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// 棋盘尺寸
#define BOARD_MARGIN 50
#define CELL_SIZE 30

// 棋子半径
#define STONE_RADIUS 14

// 颜色定义
#define BOARD_COLOR 0xD6, 0x9A, 0x45, 255  // 棋盘颜色（木色）
#define GRID_COLOR 0x00, 0x00, 0x00, 255    // 网格线颜色（黑色）
#define BLACK_STONE_COLOR 0x00, 0x00, 0x00, 255  // 黑棋颜色
#define WHITE_STONE_COLOR 0xFF, 0xFF, 0xFF, 255  // 白棋颜色
#define TEXT_COLOR 0x00, 0x00, 0x00, 255    // 文字颜色（黑色）

// SDL相关变量
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
bool isRunning = true; // 全局变量，用于控制游戏循环

// 初始化图形界面
void initUI(GameState* game) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        exit(1);
    }
    
    // 初始化SDL_ttf
    if (TTF_Init() < 0) {
        printf("TTF初始化失败: %s\n", TTF_GetError());
        SDL_Quit();
        exit(1);
    }
    
    // 创建窗口
    window = SDL_CreateWindow(
        "围棋游戏",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (window == NULL) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    
    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    
    // 加载字体
    // 尝试加载中文字体
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\msyh.ttf",
        "C:\\Windows\\Fonts\\simsun.ttc",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    for (int i = 0; i < sizeof(fontPaths)/sizeof(fontPaths[0]); i++) {
        font = TTF_OpenFont(fontPaths[i], 16);
        if (font != NULL) {
            printf("成功加载字体: %s\n", fontPaths[i]);
            break;
        }
        printf("字体加载失败(%s): %s\n", fontPaths[i], TTF_GetError());
    }
    
    if (font == NULL) {
        printf("所有字体加载尝试均失败，请确保系统中有中文字体文件\n");
        exit(1);
    }
    
    // 设置随机数种子（用于AI）
    srand((unsigned int)time(NULL));
    
    // 初始绘制
    drawBoard(game);
    displayGameInfo(game);
    SDL_RenderPresent(renderer);
}

// 清理图形界面资源
void cleanupUI() {
    if (font != NULL) {
        TTF_CloseFont(font);
    }
    
    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
    }
    
    if (window != NULL) {
        SDL_DestroyWindow(window);
    }
    
    TTF_Quit();
    SDL_Quit();
}

// 绘制棋盘
void drawBoard(GameState* game) {
    // 清屏
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    
    // 绘制棋盘背景
    SDL_Rect boardRect = {
        BOARD_MARGIN - 10,
        BOARD_MARGIN - 10,
        CELL_SIZE * (BOARD_SIZE - 1) + 20,
        CELL_SIZE * (BOARD_SIZE - 1) + 20
    };
    
    SDL_SetRenderDrawColor(renderer, BOARD_COLOR);
    SDL_RenderFillRect(renderer, &boardRect);
    
    // 绘制网格线
    SDL_SetRenderDrawColor(renderer, GRID_COLOR);
    
    // 横线
    for (int i = 0; i < BOARD_SIZE; i++) {
        SDL_RenderDrawLine(
            renderer,
            BOARD_MARGIN,
            BOARD_MARGIN + i * CELL_SIZE,
            BOARD_MARGIN + (BOARD_SIZE - 1) * CELL_SIZE,
            BOARD_MARGIN + i * CELL_SIZE
        );
    }
    
    // 竖线
    for (int i = 0; i < BOARD_SIZE; i++) {
        SDL_RenderDrawLine(
            renderer,
            BOARD_MARGIN + i * CELL_SIZE,
            BOARD_MARGIN,
            BOARD_MARGIN + i * CELL_SIZE,
            BOARD_MARGIN + (BOARD_SIZE - 1) * CELL_SIZE
        );
    }
    
    // 绘制天元和星位
    int starPoints[9][2] = {
        {3, 3}, {3, 9}, {3, 15},
        {9, 3}, {9, 9}, {9, 15},
        {15, 3}, {15, 9}, {15, 15}
    };
    
    for (int i = 0; i < 9; i++) {
        int x = starPoints[i][0];
        int y = starPoints[i][1];
        
        SDL_Rect rect = {
            BOARD_MARGIN + x * CELL_SIZE - 3,
            BOARD_MARGIN + y * CELL_SIZE - 3,
            6, 6
        };
        
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // 绘制棋子
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (game->board[y][x] != EMPTY) {
                drawStone(x, y, game->board[y][x]);
            }
        }
    }
}

// 绘制棋子
void drawStone(int x, int y, StoneColor color) {
    int screenX = BOARD_MARGIN + x * CELL_SIZE;
    int screenY = BOARD_MARGIN + y * CELL_SIZE;
    
    // 设置颜色
    if (color == BLACK) {
        SDL_SetRenderDrawColor(renderer, BLACK_STONE_COLOR);
    } else {
        SDL_SetRenderDrawColor(renderer, WHITE_STONE_COLOR);
    }
    
    // 绘制圆形
    for (int w = -STONE_RADIUS; w <= STONE_RADIUS; w++) {
        for (int h = -STONE_RADIUS; h <= STONE_RADIUS; h++) {
            if (w*w + h*h <= STONE_RADIUS*STONE_RADIUS) {
                SDL_RenderDrawPoint(renderer, screenX + w, screenY + h);
            }
        }
    }
    
    // 如果是白棋，添加黑色边框
    if (color == WHITE) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int angle = 0; angle < 360; angle++) {
            int w = (int)(STONE_RADIUS * cos(angle * 3.14159 / 180));
            int h = (int)(STONE_RADIUS * sin(angle * 3.14159 / 180));
            SDL_RenderDrawPoint(renderer, screenX + w, screenY + h);
        }
    }
}

// 显示文本
void renderText(const char* text, int x, int y) {
    if (font == NULL) {
        return;
    }
    
    SDL_Color color = {TEXT_COLOR};
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
    
    if (surface == NULL) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    if (texture == NULL) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// 显示游戏状态信息
void displayGameInfo(GameState* game) {
    char buffer[100];
    
    // 显示当前玩家
    sprintf(buffer, "当前玩家: %s", (game->currentPlayer == BLACK) ? "黑棋" : "白棋");
    renderText(buffer, BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN);
    
    // 显示提子数
    sprintf(buffer, "黑方提子: %d", game->blackCaptures);
    renderText(buffer, BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 30);
    
    sprintf(buffer, "白方提子: %d", game->whiteCaptures);
    renderText(buffer, BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 60);
    
    // 显示操作提示
    renderText("操作说明:", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 100);
    renderText("鼠标左键 - 落子", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 130);
    renderText("U键 - 悔棋", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 160);
    renderText("R键 - 重做", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 190);
    renderText("P键 - 复盘", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 220);
    renderText("A键 - AI落子", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 250);
    renderText("ESC键 - 退出", BOARD_MARGIN + CELL_SIZE * BOARD_SIZE + 20, BOARD_MARGIN + 280);
}

// 显示提示信息
void showMessage(const char* message) {
    // 在底部显示消息
    SDL_Rect msgRect = {0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30};
    
    // 清除原有消息区域
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &msgRect);
    
    // 显示新消息
    renderText(message, 10, WINDOW_HEIGHT - 25);
    SDL_RenderPresent(renderer);
}

// 处理鼠标点击
void handleMouseClick(GameState* game, int mouseX, int mouseY) {
    // 计算棋盘坐标
    int boardX = (mouseX - BOARD_MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    int boardY = (mouseY - BOARD_MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    
    // 检查是否在棋盘范围内
    if (boardX >= 0 && boardX < BOARD_SIZE && boardY >= 0 && boardY < BOARD_SIZE) {
        // 尝试落子
        if (placeStone(game, boardX, boardY)) {
            // 落子成功，重绘棋盘
            drawBoard(game);
            displayGameInfo(game);
            SDL_RenderPresent(renderer);
        } else {
            // 落子失败，显示提示
            showMessage("无效的落子位置！");
        }
    }
}

// 处理用户输入
void handleInput(GameState* game) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseClick(game, event.button.x, event.button.y);
                }
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                        
                    case SDLK_u: // 悔棋
                        if (undoMove(game)) {
                            drawBoard(game);
                            displayGameInfo(game);
                            SDL_RenderPresent(renderer);
                            showMessage("悔棋成功");
                        } else {
                            showMessage("无法悔棋");
                        }
                        break;
                        
                    case SDLK_r: // 重做
                        if (redoMove(game)) {
                            drawBoard(game);
                            displayGameInfo(game);
                            SDL_RenderPresent(renderer);
                            showMessage("重做成功");
                        } else {
                            showMessage("无法重做");
                        }
                        break;
                        
                    case SDLK_p: // 复盘
                        replayGame(game);
                        drawBoard(game);
                        displayGameInfo(game);
                        SDL_RenderPresent(renderer);
                        showMessage("复盘完成");
                        break;
                        
                    case SDLK_a: // AI落子
                        if (game->currentPlayer == WHITE) { // 假设AI是白方
                            Position aiMove = aiMakeMove(game);
                            if (aiMove.x != -1 && aiMove.y != -1) {
                                placeStone(game, aiMove.x, aiMove.y);
                                drawBoard(game);
                                displayGameInfo(game);
                                SDL_RenderPresent(renderer);
                                showMessage("AI已落子");
                            } else {
                                showMessage("AI无法找到有效的落子位置");
                            }
                        }
                        break;
                }
                break;
        }
    }
}

// 处理游戏循环中的UI更新（由go_game.c中的runGameLoop调用）
void updateGameUI(GameState* game) {
    // 处理用户输入
    handleInput(game);
    
    // 控制帧率
    SDL_Delay(16); // 约60FPS
}