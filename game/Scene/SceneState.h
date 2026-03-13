#pragma once
#include <memory>
#include <functional>
#include "../include/engine.h"

namespace Game {

class SceneState {
public:
    virtual ~SceneState() = default;
    virtual void Enter() {}
    virtual void Update(float deltaTime) = 0;
    virtual void Render(RenderCore::RenderEngine& engine) = 0;
    virtual void Exit() {}
    
    using StateChangeCallback = std::function<void(const std::string& newState)>;
    void SetStateChangeCallback(StateChangeCallback callback) {
        stateChangeCallback_ = callback;
    }
    
protected:
    void ChangeState(const std::string& newState) {
        if (stateChangeCallback_) {
            stateChangeCallback_(newState);
        }
    }
    
private:
    StateChangeCallback stateChangeCallback_;
};

} // namespace Game