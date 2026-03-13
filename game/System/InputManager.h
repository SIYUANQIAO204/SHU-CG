#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL3/SDL.h>
#include "vector.h"

namespace Game {

class InputManager {
private:
    static constexpr int KEY_COUNT = 256;
    bool keys_[KEY_COUNT] = {false};
    bool prevKeys_[KEY_COUNT] = {false};
    RenderCore::Vector2f mousePos_;
    
public:
    // 单例模式
    static InputManager& Instance() {
        static InputManager instance;
        return instance;
    }
    
    // 处理SDL事件
    void ProcessEvent(const SDL_Event& event);
    
    // 按键检测
    bool IsKeyPressed(int key) const { 
        return key >= 0 && key < KEY_COUNT ? keys_[key] : false; 
    }
    
    // 鼠标位置
    const RenderCore::Vector2f& GetMousePos() const { return mousePos_; }

    bool IsKeyJustPressed(int key) const;   // 刚刚被按下
    void NextFrame();
    void ClearAll(); // 清空所有状态
    
private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
};

} // namespace Game

#endif // INPUT_MANAGER_H