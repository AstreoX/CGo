/**
 * @file gui.c
 * @brief 围棋游戏图形界面实现
 */

#include "../include/gui.h"
#include "../include/utils.h"
#include <SDL2/SDL_ttf.h>

// 函数声明
static void renderText(SDL_Renderer* renderer, const char* text, int x, int y, TTF_Font* font, SDL_Color color);

// 颜色定义
static const SDL_Color BOARD_COLOR = {220, 179, 92, 255};  // 棋盘颜色
static const SDL_Color LINE_COLOR = {0, 0, 0, 255};        // 线条颜色
static const SDL_Color BLACK_STONE_COLOR = {0, 0, 0, 255}; // 黑棋颜色
static const SDL_Color WHITE_STONE_COLOR = {255, 255, 255, 255}; // 白棋颜色
static const SDL_Color TEXT_COLOR = {0, 0, 0, 255};        // 文本颜色
static const SDL_Color ERROR_COLOR = {255, 0, 0, 255};     // 错误提示颜色
static const SDL_Color HINT_COLOR = {0, 128, 0, 255};      // 提示颜色

// 字体大小
static const int FONT_SIZE_SMALL = 14;
static const int FONT_SIZE_MEDIUM = 18;
static const int FONT_SIZE_LARGE = 24;

// 全局字体
static TTF_Font* smallFont = NULL;
static TTF_Font* mediumFont = NULL;
static TTF_Font* largeFont = NULL;

bool initGUI(GUI* gui) {
    // 创建窗口
    gui->window = SDL_CreateWindow("围棋游戏", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gui->window) {
        LOG_ERROR("无法创建窗口");
        printf("SDL错误: %s\n", SDL_GetError());
        return false;
    }
    
    // 创建渲染器
    gui->renderer = SDL_CreateRenderer(gui->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gui->renderer) {
        LOG_ERROR("无法创建渲染器");
        printf("SDL错误: %s\n", SDL_GetError());
        SDL_DestroyWindow(gui->window);
        return false;
    }
    
    // 初始化TTF
    if (TTF_Init() == -1) {
        LOG_ERROR("无法初始化SDL_ttf");
        printf("SDL_ttf错误: %s\n", TTF_GetError());
        SDL_DestroyRenderer(gui->renderer);
        SDL_DestroyWindow(gui->window);
        return false;
    }
    
    // 加载字体
    // 注意：在实际应用中，应该提供一个默认字体文件
    // 这里假设系统中有这些字体，实际使用时应该将字体文件放在resources目录下
    smallFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", FONT_SIZE_SMALL);
    mediumFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", FONT_SIZE_MEDIUM);
    largeFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", FONT_SIZE_LARGE);
    
    if (!smallFont || !mediumFont || !largeFont) {
        LOG_ERROR("无法加载字体");
        printf("SDL_ttf错误: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(gui->renderer);
        SDL_DestroyWindow(gui->window);
        return false;
    }
    
    // 初始化纹理指针为NULL
    gui->logoTexture = NULL;
    gui->backgroundTexture = NULL;
    
    // 加载LOGO图片
    if (fileExists(LOGO_FILE)) {
        SDL_Surface* logoSurface = IMG_Load(LOGO_FILE);
        if (logoSurface) {
            gui->logoTexture = SDL_CreateTextureFromSurface(gui->renderer, logoSurface);
            SDL_FreeSurface(logoSurface);
            //printf("成功加载LOGO图片: %s\n", LOGO_FILE);
        } else {
            printf("无法加载LOGO图片: %s (错误: %s)\n", LOGO_FILE, IMG_GetError());
        }
    } else {
        printf("LOGO文件不存在: %s\n", LOGO_FILE);
    }
    
    // 加载背景图片
    if (fileExists(BACKGROUND_FILE)) {
        SDL_Surface* bgSurface = IMG_Load(BACKGROUND_FILE);
        if (bgSurface) {
            gui->backgroundTexture = SDL_CreateTextureFromSurface(gui->renderer, bgSurface);
            SDL_FreeSurface(bgSurface);
            //printf("成功加载背景图片: %s\n", BACKGROUND_FILE);
        } else {
            printf("无法加载背景图片: %s (错误: %s)\n", BACKGROUND_FILE, IMG_GetError());
        }
    } else {
        printf("背景图片文件不存在: %s\n", BACKGROUND_FILE);
    }
    
    // 设置各区域位置和大小
    gui->logoRect.x = 730;
    gui->logoRect.y = 10;
    gui->logoRect.w = 60;
    gui->logoRect.h = 60;
    
    gui->boardRect.x = BOARD_MARGIN;
    gui->boardRect.y = BOARD_MARGIN + 60; // 留出空间放logo
    gui->boardRect.w = CELL_SIZE * (BOARD_SIZE - 1);
    gui->boardRect.h = CELL_SIZE * (BOARD_SIZE - 1);
    
    gui->statusRect.x = BOARD_MARGIN + gui->boardRect.w + 20;
    gui->statusRect.y = BOARD_MARGIN + 60;
    gui->statusRect.w = WINDOW_WIDTH - gui->statusRect.x - 20;
    gui->statusRect.h = 150;
    
    gui->violationRect.x = BOARD_MARGIN;
    gui->violationRect.y = BOARD_MARGIN + gui->boardRect.h + 70;
    gui->violationRect.w = gui->boardRect.w;
    gui->violationRect.h = 30;
    
    gui->controlsRect.x = BOARD_MARGIN + gui->boardRect.w + 20;
    gui->controlsRect.y = BOARD_MARGIN + gui->statusRect.h + 80;
    gui->controlsRect.w = gui->statusRect.w;
    gui->controlsRect.h = 200;
    
    return true;
}

void freeGUI(GUI* gui) {
    // 释放纹理
    if (gui->logoTexture) {
        SDL_DestroyTexture(gui->logoTexture);
    }
    
    if (gui->backgroundTexture) {
        SDL_DestroyTexture(gui->backgroundTexture);
    }
    
    // 释放字体
    if (smallFont) TTF_CloseFont(smallFont);
    if (mediumFont) TTF_CloseFont(mediumFont);
    if (largeFont) TTF_CloseFont(largeFont);
    
    // 释放渲染器和窗口
    SDL_DestroyRenderer(gui->renderer);
    SDL_DestroyWindow(gui->window);
    
    // 退出TTF
    TTF_Quit();
}

void renderGame(GUI* gui, Game* game) {
    // 清空渲染器
    SDL_SetRenderDrawColor(gui->renderer, 240, 240, 240, 255);
    SDL_RenderClear(gui->renderer);
    
    // 渲染背景图片（如果存在）
    if (gui->backgroundTexture) {
        // 创建一个覆盖整个窗口的矩形
        SDL_Rect backgroundRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        // 稍微降低背景图片的不透明度，使其不影响游戏视觉
        SDL_SetTextureAlphaMod(gui->backgroundTexture, 128); // 50%透明度
        SDL_RenderCopy(gui->renderer, gui->backgroundTexture, NULL, &backgroundRect);
        // 恢复正常不透明度
        SDL_SetTextureAlphaMod(gui->backgroundTexture, 255);
    }
    
    // 渲染logo
    if (gui->logoTexture) {
        SDL_RenderCopy(gui->renderer, gui->logoTexture, NULL, &gui->logoRect);
    }
    
    // 渲染棋盘
    renderBoard(gui, game);
    
    // 渲染状态信息
    renderStatus(gui, game);
    
    // 渲染控制提示
    renderControls(gui, game);
    
    // 如果游戏结束，渲染游戏结束界面
    if (game->state == STATE_GAMEOVER) {
        renderGameOver(gui, game);
    }
    
    // 更新屏幕
    SDL_RenderPresent(gui->renderer);
}

void renderBoard(GUI* gui, Game* game) {
    // 先绘制半透明的白色背景，使棋盘在背景图上更清晰
    SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, 220);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    
    // 创建一个比棋盘稍大的背景矩形
    SDL_Rect boardBackground = {
        gui->boardRect.x - 15, 
        gui->boardRect.y - 15, 
        gui->boardRect.w + 30, 
        gui->boardRect.h + 30
    };
    SDL_RenderFillRect(gui->renderer, &boardBackground);
    
    // 恢复正常绘制模式
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
    
    // 绘制棋盘背景
    SDL_SetRenderDrawColor(gui->renderer, BOARD_COLOR.r, BOARD_COLOR.g, BOARD_COLOR.b, BOARD_COLOR.a);
    SDL_RenderFillRect(gui->renderer, &gui->boardRect);
    
    // 绘制棋盘网格线
    SDL_SetRenderDrawColor(gui->renderer, LINE_COLOR.r, LINE_COLOR.g, LINE_COLOR.b, LINE_COLOR.a);
    
    // 横线
    for (int i = 0; i < BOARD_SIZE; i++) {
        int y = gui->boardRect.y + i * CELL_SIZE;
        SDL_RenderDrawLine(gui->renderer, 
                          gui->boardRect.x, y, 
                          gui->boardRect.x + gui->boardRect.w, y);
    }
    
    // 竖线
    for (int i = 0; i < BOARD_SIZE; i++) {
        int x = gui->boardRect.x + i * CELL_SIZE;
        SDL_RenderDrawLine(gui->renderer, 
                          x, gui->boardRect.y, 
                          x, gui->boardRect.y + gui->boardRect.h);
    }
    
    // 绘制天元和星位
    int starPoints[3] = {3, 9, 15}; // 3-4线、天元、15-16线
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int x = gui->boardRect.x + starPoints[i] * CELL_SIZE;
            int y = gui->boardRect.y + starPoints[j] * CELL_SIZE;
            
            SDL_Rect rect = {x - 3, y - 3, 6, 6};
            SDL_RenderFillRect(gui->renderer, &rect);
        }
    }
    
    // 绘制棋子
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (game->board.board[y][x] != EMPTY) {
                int screenX = gui->boardRect.x + x * CELL_SIZE;
                int screenY = gui->boardRect.y + y * CELL_SIZE;
                int radius = CELL_SIZE / 2 - 2;
                
                // 设置颜色
                if (game->board.board[y][x] == BLACK) {
                    SDL_SetRenderDrawColor(gui->renderer, 
                                          BLACK_STONE_COLOR.r, 
                                          BLACK_STONE_COLOR.g, 
                                          BLACK_STONE_COLOR.b, 
                                          BLACK_STONE_COLOR.a);
                } else {
                    SDL_SetRenderDrawColor(gui->renderer, 
                                          WHITE_STONE_COLOR.r, 
                                          WHITE_STONE_COLOR.g, 
                                          WHITE_STONE_COLOR.b, 
                                          WHITE_STONE_COLOR.a);
                }
                
                // 绘制圆形棋子（使用多边形近似）
                for (int i = 0; i < radius; i++) {
                    for (int j = 0; j < radius; j++) {
                        if (i*i + j*j <= radius*radius) {
                            SDL_RenderDrawPoint(gui->renderer, screenX + i, screenY + j);
                            SDL_RenderDrawPoint(gui->renderer, screenX - i, screenY + j);
                            SDL_RenderDrawPoint(gui->renderer, screenX + i, screenY - j);
                            SDL_RenderDrawPoint(gui->renderer, screenX - i, screenY - j);
                        }
                    }
                }
                
                // 标记最后一步落子
                if (x == game->board.lastMove.x && y == game->board.lastMove.y) {
                    SDL_SetRenderDrawColor(gui->renderer, 
                                          255 - BLACK_STONE_COLOR.r, 
                                          255 - BLACK_STONE_COLOR.g, 
                                          255 - BLACK_STONE_COLOR.b, 
                                          BLACK_STONE_COLOR.a);
                    SDL_Rect rect = {screenX - 3, screenY - 3, 6, 6};
                    SDL_RenderFillRect(gui->renderer, &rect);
                }
            }
        }
    }
}

void renderStatus(GUI* gui, Game* game) {
    // 绘制状态区域背景（半透明）
    SDL_SetRenderDrawColor(gui->renderer, 230, 230, 230, 220);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(gui->renderer, &gui->statusRect);
    
    // 恢复正常绘制模式
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
    
    // 准备状态文本
    char statusText[256];
    
    // 游戏模式
    const char* modeText = (game->mode == MODE_PVP) ? "人人对战" : "人机对战";
    
    // 当前玩家
    const char* playerText = (game->board.currentPlayer == BLACK) ? "黑方行棋" : "白方行棋";
    
    // 黑方提子数
    sprintf(statusText, "黑方气数: %d", game->board.blackLiberties);
    renderText(gui->renderer, statusText, gui->statusRect.x + 10, gui->statusRect.y + 10, mediumFont, TEXT_COLOR);
    
    // 白方提子数
    sprintf(statusText, "白方气数: %d", game->board.whiteLiberties);
    renderText(gui->renderer, statusText, gui->statusRect.x + 10, gui->statusRect.y + 40, mediumFont, TEXT_COLOR);
    
    // 黑方提子数
    sprintf(statusText, "黑方提子数: %d", game->board.blackCaptures);
    renderText(gui->renderer, statusText, gui->statusRect.x + 10, gui->statusRect.y + 70, mediumFont, TEXT_COLOR);
    
    // 白方提子数
    sprintf(statusText, "白方提子数: %d", game->board.whiteCaptures);
    renderText(gui->renderer, statusText, gui->statusRect.x + 10, gui->statusRect.y + 100, mediumFont, TEXT_COLOR);
    
    // 游戏模式和当前玩家
    sprintf(statusText, "%s - %s", modeText, playerText);
    renderText(gui->renderer, statusText, gui->statusRect.x + 10, gui->statusRect.y + 130, mediumFont, TEXT_COLOR);
    
    // 如果游戏结束，显示胜者
    if (game->state == STATE_GAMEOVER) {
        const char* winnerText = (game->winner == BLACK) ? "黑方胜利!" : 
                               (game->winner == WHITE) ? "白方胜利!" : "平局!";
        renderText(gui->renderer, winnerText, 
                  WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 20, 
                  largeFont, ERROR_COLOR);
    }
}

void renderViolationHint(GUI* gui, Game* game, const char* message) {
    (void)game; // 标记参数已使用
    if (!message) return;
    
    // 绘制违规提示区域背景（半透明）
    SDL_SetRenderDrawColor(gui->renderer, 255, 220, 220, 220);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(gui->renderer, &gui->violationRect);
    
    // 恢复正常绘制模式
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
    
    // 渲染提示文本
    renderText(gui->renderer, message, 
              gui->violationRect.x + 10, gui->violationRect.y + 5, 
              mediumFont, ERROR_COLOR);
}

void renderControls(GUI* gui, Game* game) {
    (void)game; // 标记参数已使用
    // 绘制控制提示区域背景（半透明）
    SDL_SetRenderDrawColor(gui->renderer, 220, 240, 220, 220);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(gui->renderer, &gui->controlsRect);
    
    // 恢复正常绘制模式
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
    
    // 渲染控制提示文本
    renderText(gui->renderer, "[A] AI落子（当前方）", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 10, 
              mediumFont, HINT_COLOR);
    
    renderText(gui->renderer, "[M] 切换游戏模式", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 40, 
              mediumFont, HINT_COLOR);
    
    renderText(gui->renderer, "[U] 悔棋", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 70, 
              mediumFont, HINT_COLOR);
    
    renderText(gui->renderer, "[P] 回溯棋局", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 100, 
              mediumFont, HINT_COLOR);
    
    renderText(gui->renderer, "[T] 提示开/关", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 130, 
              mediumFont, HINT_COLOR);
    
    renderText(gui->renderer, "[E] 结束游戏", 
              gui->controlsRect.x + 10, gui->controlsRect.y + 160, 
              mediumFont, HINT_COLOR);
}

/**
 * @brief 渲染文本
 * @param renderer SDL渲染器
 * @param text 文本内容
 * @param x X坐标
 * @param y Y坐标
 * @param font 字体
 * @param color 颜色
 */
static void renderText(SDL_Renderer* renderer, const char* text, int x, int y, TTF_Font* font, SDL_Color color) {
    if (!text || !font) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

bool screenToBoardPos(GUI* gui, int screenX, int screenY, Position* boardPos) {
    // 检查是否在棋盘范围内
    if (screenX < gui->boardRect.x - CELL_SIZE/2 || 
        screenX > gui->boardRect.x + gui->boardRect.w + CELL_SIZE/2 || 
        screenY < gui->boardRect.y - CELL_SIZE/2 || 
        screenY > gui->boardRect.y + gui->boardRect.h + CELL_SIZE/2) {
        return false;
    }
    
    // 计算最接近的交叉点
    int boardX = (screenX - gui->boardRect.x + CELL_SIZE/2) / CELL_SIZE;
    int boardY = (screenY - gui->boardRect.y + CELL_SIZE/2) / CELL_SIZE;
    
    // 确保在棋盘范围内
    if (boardX < 0) boardX = 0;
    if (boardX >= BOARD_SIZE) boardX = BOARD_SIZE - 1;
    if (boardY < 0) boardY = 0;
    if (boardY >= BOARD_SIZE) boardY = BOARD_SIZE - 1;
    
    // 设置输出
    boardPos->x = boardX;
    boardPos->y = boardY;
    
    return true;
}

bool handleEvents(GUI* gui, Game* game) {
    SDL_Event event;
    static const char* violationMessage = NULL;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    Position boardPos;
                    if (screenToBoardPos(gui, event.button.x, event.button.y, &boardPos)) {
                        // 检查落子是否合法
                        violationMessage = getViolationHint(game, boardPos);
                        
                        if (!violationMessage) {
                            // 尝试落子
                            handlePlayerMove(game, boardPos);
                        } else {
                            // 渲染违规提示
                            renderViolationHint(gui, game, violationMessage);
                            SDL_RenderPresent(gui->renderer);
                        }
                    }
                }
                break;
                
            case SDL_KEYDOWN:
                // 如果游戏已结束，只有按下空格键或回车键才重新开始游戏
                if (game->state == STATE_GAMEOVER) {
                    if (event.key.keysym.sym == SDLK_SPACE || 
                        event.key.keysym.sym == SDLK_RETURN) {
                        // 初始化新游戏
                        initGame(game);
                    }
                    return true;
                }
                
                switch (event.key.keysym.sym) {
                    case SDLK_a: // AI落子（当前玩家）
                        if (!game->aiThinking && game->state == STATE_PLAYING) {
                            makeAIMoveForCurrentPlayer(game);
                        }
                        break;
                        
                    case SDLK_m: // 切换AI模式
                        toggleGameMode(game);
                        break;
                        
                    case SDLK_u: // 悔棋
                        handleUndo(game);
                        break;
                        
                    case SDLK_p: // 前进
                        handleRedo(game);
                        break;
                        
                    case SDLK_t: // 切换提示
                        toggleHints(game);
                        break;
                        
                    case SDLK_e: // 结束游戏
                        if (game->state == STATE_PLAYING) {
                            endGameManually(game);
                        }
                        break;
                        
                    case SDLK_ESCAPE: // 退出
                        return false;
                }
                break;
        }
    }
    
    return true;
}

/**
 * @brief 渲染游戏结束界面
 * @param gui GUI指针
 * @param game 游戏指针
 */
void renderGameOver(GUI* gui, Game* game) {
    // 半透明黑色背景
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 180);
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(gui->renderer, &overlay);
    
    // 游戏结束面板背景
    SDL_Rect panelRect = {WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2 - 180, 400, 360};
    SDL_SetRenderDrawColor(gui->renderer, 240, 240, 240, 240);
    SDL_RenderFillRect(gui->renderer, &panelRect);
    
    // 边框
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(gui->renderer, &panelRect);
    
    // 标题
    renderText(gui->renderer, "游戏结束", 
              panelRect.x + 150, panelRect.y + 20, 
              largeFont, TEXT_COLOR);
    
    // 获取胜者
    const char* winnerText = (game->winner == BLACK) ? "黑方胜利!" : 
                           (game->winner == WHITE) ? "白方胜利!" : "平局!";
    
    // 高亮显示胜者
    SDL_Color winnerColor = (game->winner == BLACK) ? BLACK_STONE_COLOR : 
                          (game->winner == WHITE) ? WHITE_STONE_COLOR : TEXT_COLOR;
    
    renderText(gui->renderer, winnerText, 
              panelRect.x + 140, panelRect.y + 60, 
              largeFont, winnerColor);
    
    // 计算详细得分
    char scoreInfo[256];
    
    // 计算各方占据的交叉点和地盘
    int blackStones = 0;
    int whiteStones = 0;
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (game->board.board[y][x] == BLACK) {
                blackStones++;
            } else if (game->board.board[y][x] == WHITE) {
                whiteStones++;
            }
        }
    }
    
    // 黑方得分信息
    sprintf(scoreInfo, "黑方棋子: %d", blackStones);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 110, 
              mediumFont, TEXT_COLOR);
    
    sprintf(scoreInfo, "黑方提子: %d", game->board.whiteCaptures);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 140, 
              mediumFont, TEXT_COLOR);
    
    int blackTotal = blackStones + game->board.whiteCaptures;
    sprintf(scoreInfo, "黑方总分: %d", blackTotal);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 170, 
              mediumFont, TEXT_COLOR);
    
    // 白方得分信息
    sprintf(scoreInfo, "白方棋子: %d", whiteStones);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 210, 
              mediumFont, TEXT_COLOR);
    
    sprintf(scoreInfo, "白方提子: %d", game->board.blackCaptures);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 240, 
              mediumFont, TEXT_COLOR);
    
    sprintf(scoreInfo, "贴目: +4");
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 270, 
              mediumFont, TEXT_COLOR);
    
    int whiteTotal = whiteStones + game->board.blackCaptures + 4; // 加上贴目
    sprintf(scoreInfo, "白方总分: %d", whiteTotal);
    renderText(gui->renderer, scoreInfo, 
              panelRect.x + 50, panelRect.y + 300, 
              mediumFont, TEXT_COLOR);
    
    // 操作提示
    renderText(gui->renderer, "按空格或回车继续...", 
              panelRect.x + 135, panelRect.y + 330, 
              smallFont, HINT_COLOR);
    
    // 恢复正常绘制模式
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
}