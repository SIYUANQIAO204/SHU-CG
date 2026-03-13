#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "engine.h"
namespace Game {

class SceneState;
class SceneManager {
public:
    static SceneManager& Instance();
    
    void RegisterState(const std::string& name, std::unique_ptr<SceneState> state);
    void ChangeState(const std::string& name);
    
    void Update(float deltaTime);
    void Render(class RenderCore::RenderEngine& engine);
    
    bool ShouldQuit() const { return shouldQuit_; }
    
private:
    SceneManager();
    void InitializeStates();
    
    std::unordered_map<std::string, std::unique_ptr<SceneState>> states_;
    SceneState* currentState_ = nullptr;
    bool shouldQuit_ = false;
};

} // namespace Game