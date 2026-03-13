#include "InputManager.h"

namespace Game {

void InputManager::ProcessEvent(const SDL_Event& event) {
    // std::cout << "Event received: type=" << event.type << std::endl; // 调试
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            // std::cout << "Key DOWN scancode: " << event.key.scancode << std::endl; 
            if (event.key.scancode < KEY_COUNT) {
                keys_[event.key.scancode] = true;
            }
            break;
        case SDL_EVENT_KEY_UP:
            // std::cout << "Key UP scancode: " << event.key.scancode << std::endl;
            if (event.key.scancode < KEY_COUNT) {
                keys_[event.key.scancode] = false;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            mousePos_[0] = static_cast<float>(event.motion.x);
            mousePos_[1] = static_cast<float>(event.motion.y);
            break;
    }
}

bool InputManager::IsKeyJustPressed(int key) const {
    return key >= 0 && key < KEY_COUNT ? (keys_[key] && !prevKeys_[key]) : false;
}
void InputManager::NextFrame() { // 每帧最后调用
    for (int i = 0; i < KEY_COUNT; ++i) prevKeys_[i] = keys_[i];
}
void InputManager::ClearAll() {
    for (int i = 0; i < KEY_COUNT; ++i) { keys_[i] = false; prevKeys_[i] = false; }
}

} // namespace Game