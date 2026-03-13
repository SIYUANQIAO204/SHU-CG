#include "SceneManager.h"
#include "SceneState.h"
#include "StartScene.h"
#include "GameSceneWrapper.h"
#include <iostream>
#include "../System/InputManager.h"

namespace Game {

SceneManager& SceneManager::Instance() {
    static SceneManager instance;
    return instance;
}

SceneManager::SceneManager() {
    InitializeStates();
}

void SceneManager::InitializeStates() {
    // 只注册开始场景，游戏场景在需要时动态创建
    auto startScene = std::make_unique<StartScene>();
    startScene->SetStateChangeCallback([this](const std::string& newState) {
        ChangeState(newState);
    });
    RegisterState("Start", std::move(startScene));
    
    // 设置初始状态
    ChangeState("Start");
}

void SceneManager::RegisterState(const std::string& name, std::unique_ptr<SceneState> state) {
    states_[name] = std::move(state);
}

void SceneManager::ChangeState(const std::string& name) {
    if (name == "Exit") {
        shouldQuit_ = true;
        return;
    }
    
    // 特殊处理游戏场景：每次都重新创建
    if (name == "Game") {
        // 创建新的游戏场景实例
        auto gameScene = std::make_unique<GameSceneWrapper>();
        gameScene->SetStateChangeCallback([this](const std::string& newState) {
            ChangeState(newState);
        });
        
        if (currentState_) {
            currentState_->Exit();
        }
        
        currentState_ = gameScene.get();
        states_["Game"] = std::move(gameScene);
        currentState_->Enter();
        
        std::cout << "Created new Game Scene" << std::endl;
        return;
    }
    
    // 其他场景正常切换
    auto it = states_.find(name);
    if (it != states_.end()) {
        if (currentState_) {
            currentState_->Exit();
        }
        
        currentState_ = it->second.get();
        currentState_->Enter();
        
        std::cout << "Changed to state: " << name << std::endl;
    }
}

void SceneManager::Update(float deltaTime) {
    if (currentState_) {
        currentState_->Update(deltaTime);
    }
}

void SceneManager::Render(RenderCore::RenderEngine& engine) {
    if (currentState_) {
        currentState_->Render(engine);
    }
}

} // namespace Game