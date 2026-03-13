#pragma once
#include "SceneState.h"

namespace Game {

class StartScene : public SceneState {
public:
    void Enter() override;
    void Update(float deltaTime) override;
    void Render(RenderCore::RenderEngine& engine) override;
    void Exit() override {} // 明确提供实现
    
private:
    void RenderMenu(RenderCore::RenderEngine& engine);
    void HandleInput();
    
    int selectedOption_ = 0;
    static constexpr int MENU_OPTIONS = 2;
    
    static constexpr int TITLE_Y = 150;
    static constexpr int MENU_START_Y = 300;
    static constexpr int MENU_EXIT_Y = 350;
    static constexpr int MENU_ITEM_HEIGHT = 50;
};

} // namespace Game