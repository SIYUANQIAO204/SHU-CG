#pragma once
#include "SceneState.h"
#include "GameScene.h"

namespace Game {

class GameSceneWrapper : public SceneState {
public:
    void Enter() override;
    void Update(float deltaTime) override;
    void Render(RenderCore::RenderEngine& engine) override;
    void Exit() override {} // 明确提供实现
    
private:
    void RenderHUD(RenderCore::RenderEngine& engine);
    void HandleInput();
    void CheckGameOver();
    
    GameScene gameScene_;
    int playerLives_ = 1;
    int score_ = 0;
    static constexpr int MAX_LIVES = 1;
    bool isGameOver_ = false;
};

} // namespace Game