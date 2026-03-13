#include <SDL3/SDL.h>
#include <iostream>
#include <memory>
#include "engine.h"
#include "color.h"
#include "System/InputManager.h"
#include "Scene/GameScene.h"
#include <windows.h>
#include "Scene/SceneManager.h"

// 游戏配置
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TARGET_FPS = 60;
const float FRAME_TIME = 1000.0f / TARGET_FPS; // 每帧毫秒数

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    // 初始化SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // 创建窗口
    SDL_Window* window = SDL_CreateWindow(
        "STG Game - Milestone 1",
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    
    // 创建渲染器
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (!sdlRenderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // 初始化游戏引擎
    RenderCore::RenderEngine engine;
    engine.init(SCREEN_WIDTH, SCREEN_HEIGHT);
    // 获取场景管理器实例
    auto& sceneManager = Game::SceneManager::Instance();

    
    // 游戏主循环
    bool running = true;
    uint64_t lastTime = SDL_GetTicks();
    
    std::cout << "========= Game Started =========" << std::endl;
    std::cout << "Controls: Arrow Keys to move" << std::endl;
    
    while (running && !sceneManager.ShouldQuit()) {
        // 计算帧时间
        uint64_t currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // 事件处理
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            Game::InputManager::Instance().ProcessEvent(event);
            
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        
        // 更新游戏逻辑
        engine.clear(); // 清空画布
        sceneManager.Update(deltaTime);
        sceneManager.Render(engine);
        
        // 渲染到屏幕
        engine.render();
        auto buffer = engine.get_frame_buffer();
        
        // 创建SDL纹理
        SDL_Texture* texture = SDL_CreateTexture(
            sdlRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH, SCREEN_HEIGHT
        );
        
        // 更新纹理数据
        SDL_UpdateTexture(texture, nullptr, buffer.data(), SCREEN_WIDTH * 4);
        
        // 渲染纹理
        SDL_RenderClear(sdlRenderer);
        SDL_RenderTexture(sdlRenderer, texture, nullptr, nullptr);
        SDL_RenderPresent(sdlRenderer);
        
        // 释放纹理
        SDL_DestroyTexture(texture);
        
        // 帧率控制
        uint64_t frameEnd = SDL_GetTicks();
        float elapsed = frameEnd - currentTime;
        if (elapsed < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - elapsed);
        }
        Game::InputManager::Instance().NextFrame();
    }
    
    // 清理资源
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "========= Game Exited =========" << std::endl;
    return 0;
}