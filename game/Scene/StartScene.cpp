#include "StartScene.h"
#include "../System/InputManager.h"
#include "../include/engine.h"
#include "../include/color.h"
#include "../include/rectangle.h"
#include "../include/polygon.h"
#include "../Utils/GameFont.h"
#include <iostream>

namespace Game {

void StartScene::Enter() {
    Game::InputManager::Instance().ClearAll();
    std::cout << "Entering Start Scene" << std::endl;
    selectedOption_ = 0;
}

void StartScene::Update(float deltaTime) {
    HandleInput();
}

void StartScene::Render(RenderCore::RenderEngine& engine) {
    // 使用预定义的黑色背景
    engine.set_global_options({
        .background_color = RenderCore::Colors::Black
    });
    
    // 绘制标题
    GameFont::Instance().RenderText(engine, "STG GAME", {250, 100}, RenderCore::Colors::Yellow, 2.0f);
    
    // 绘制装饰线
    engine.set_pen_options({.color = RenderCore::Colors::Blue});
    RenderCore::Rectangle line1{{200, 180}, {600, 182}};
    RenderCore::Rectangle line2{{200, 184}, {600, 186}};
    engine.add_primitive(line1);
    engine.add_primitive(line2);
    
    // 绘制菜单
    RenderMenu(engine);
}

void StartScene::RenderMenu(RenderCore::RenderEngine& engine) {
    // 开始游戏选项
    if (selectedOption_ == 0) {
        GameFont::Instance().RenderText(engine, "START GAME", {280, 300}, RenderCore::Colors::Yellow, 1.5f);
    } else {
        GameFont::Instance().RenderText(engine, "START GAME", {280, 300}, RenderCore::Colors::White, 1.5f);
    }
    
    // 退出程序选项
    if (selectedOption_ == 1) {
        GameFont::Instance().RenderText(engine, "QUIT GAME", {290, 350}, RenderCore::Colors::Yellow, 1.5f);
    } else {
        GameFont::Instance().RenderText(engine, "QUIT GAME", {290, 350}, RenderCore::Colors::White, 1.5f);
    }
    
    // 绘制选择指示器
    engine.set_pen_options({.color = RenderCore::Colors::Green, .fill_color = RenderCore::Colors::Green});
    int indicatorY = selectedOption_ == 0 ? 305 : 355;
    std::vector<RenderCore::Point> indicator = {
        {260, indicatorY}, 
        {250, indicatorY + 8}, 
        {270, indicatorY + 8}
    };
    engine.add_primitive(RenderCore::make_polygon(indicator));
    
}

void StartScene::HandleInput() {
    auto& input = InputManager::Instance();
    
    // 上下选择
    static bool keyProcessed = false;
    
    if (input.IsKeyPressed(SDL_SCANCODE_UP) && !keyProcessed) {
        selectedOption_ = (selectedOption_ - 1 + MENU_OPTIONS) % MENU_OPTIONS;
        keyProcessed = true;
    } else if (input.IsKeyPressed(SDL_SCANCODE_DOWN) && !keyProcessed) {
        selectedOption_ = (selectedOption_ + 1) % MENU_OPTIONS;
        keyProcessed = true;
    } else if (!input.IsKeyPressed(SDL_SCANCODE_UP) && !input.IsKeyPressed(SDL_SCANCODE_DOWN)) {
        keyProcessed = false;
    }
    
    // 确认选择
    if (input.IsKeyPressed(SDL_SCANCODE_RETURN) || input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (selectedOption_ == 0) {
            ChangeState("Game");
        } else {
            ChangeState("Exit");
        }
    }
}

} // namespace Game