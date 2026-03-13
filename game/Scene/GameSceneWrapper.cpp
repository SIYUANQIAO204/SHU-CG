#include "GameSceneWrapper.h"
#include "../System/InputManager.h"
#include "../include/engine.h"
#include "../include/color.h"
#include "../include/rectangle.h"
#include "../include/polygon.h"
#include "../Utils/GameFont.h"
#include <iostream>
#include <sstream>

namespace Game {

void GameSceneWrapper::Enter() {
    Game::InputManager::Instance().ClearAll();
    std::cout << "Entering Game Scene" << std::endl;
    
    // 完全重置游戏状态
    playerLives_ = 1;  // 重置为1条命
    score_ = 0;
    isGameOver_ = false;
    
    // 重新初始化游戏场景
    gameScene_.Init();
    
    std::cout << "Game state reset - Lives: " << playerLives_ << ", Score: " << score_ << std::endl;
}


void GameSceneWrapper::Update(float deltaTime) {
    auto &input = InputManager::Instance();
    if (isGameOver_) {
        HandleInput();
        return;
    }
    
    HandleInput();
    gameScene_.Update(deltaTime);
    
    // 直接从GameScene获取实际分数，而不是模拟增加
    score_ = gameScene_.GetScore(); // 需要确保GameScene有GetScore方法

    if (gameScene_.GetState() == GameScene::GameState::GAMEOVER) {
        isGameOver_ = true;
    }
}

void GameSceneWrapper::Render(RenderCore::RenderEngine& engine) {
    // 先渲染原始游戏场景
    gameScene_.Render(engine);
    
    // 然后叠加渲染HUD
    RenderHUD(engine);
    
    // 如果游戏结束，显示结束画面
    if (isGameOver_) {
        // 使用黑色半透明覆盖层
        engine.set_pen_options({.color = RenderCore::Colors::Black, .fill_color = RenderCore::Colors::Black});
        RenderCore::Rectangle overlay{{150, 150}, {650, 450}};
        engine.add_primitive(overlay);
        
        // 游戏结束文字
        GameFont::Instance().RenderText(engine, "GAME OVER", {280, 200}, RenderCore::Colors::Red, 2.0f);
        
        // 显示最终分数
        std::stringstream scoreText;
        scoreText << "SCORE: " << score_;
        GameFont::Instance().RenderText(engine, scoreText.str(), {320, 280}, RenderCore::Colors::Yellow, 1.2f);
        
        // 返回提示
        GameFont::Instance().RenderText(engine, "PRESS ESC TO CONTINUE", {260, 350}, RenderCore::Colors::Green, 1.0f);
    }
}

void GameSceneWrapper::RenderHUD(RenderCore::RenderEngine& engine) {
    if (isGameOver_) return;
    
    // 绘制生命数显示
    GameFont::Instance().RenderText(engine, "LIVES:", {10, 45}, RenderCore::Colors::Green, 1.5f);
    
    // 绘制分数显示 - 使用实际的游戏分数
    std::stringstream scoreText;
    scoreText << "SCORE: " << score_;
    GameFont::Instance().RenderText(engine, scoreText.str(), {600, 25}, RenderCore::Colors::Yellow, 1.5f);
    
    // 在分数下方添加目标分数提示
    GameFont::Instance().RenderText(engine, "GOAL: 2000", {600, 40}, RenderCore::Colors::Yellow, 1.5f);

}

void GameSceneWrapper::HandleInput() {
    auto& input = InputManager::Instance();
    
    // 按ESC返回开始界面
    if (input.IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        ChangeState("Start");
    }
}

void GameSceneWrapper::CheckGameOver() {
    if (isGameOver_) return;
    
    // 这里应该从GameScene中检测玩家是否死亡
    // 由于原GameScene已经有gameOver_变量，我们可以直接使用它
    // 或者通过其他方式检测玩家生命状态
    
    // 临时方案：如果分数超过一定值就减少生命（用于测试）
    // static int lifeReductionScore = 500;
    // if (score_ >= lifeReductionScore && playerLives_ > 0) {
    //     playerLives_--;
    //     lifeReductionScore += 500;
    //     std::cout << "Life lost! Remaining: " << playerLives_ << std::endl;
    // }
    
    // 生命用尽时游戏结束
    if (playerLives_ <= 0) {
        isGameOver_ = true;
        std::cout << "Game Over! Final Score: " << score_ << std::endl;
    }
    
    // 或者直接使用GameScene的gameOver_状态
    // if (gameScene_.IsGameOver()) {
    //     isGameOver_ = true;
    // }
}
} // namespace Game