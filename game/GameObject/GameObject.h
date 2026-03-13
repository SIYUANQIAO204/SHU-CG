#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "../include/engine.h"
#include "../include/vector.h"
#include "../include/rectangle.h"

namespace Game {

class GameObject {
protected:
    RenderCore::Vector2f position_;
    RenderCore::Vector2f velocity_;
    float rotation_ = 0.0f;
    bool active_ = true;
    
public:
    GameObject() = default;
    GameObject(const RenderCore::Vector2f& pos) : position_(pos) {}
    virtual ~GameObject() = default;
    
    // 核心接口
    virtual void Update(float deltaTime) = 0;
    virtual void Render(RenderCore::RenderEngine& engine) = 0;
    virtual RenderCore::Rectangle GetBounds() const = 0;
    
    // 激活状态
    bool IsActive() const { return active_; }
    void SetActive(bool active) { active_ = active; }
    
    // 位置访问
    const RenderCore::Vector2f& GetPosition() const { return position_; }
    void SetPosition(const RenderCore::Vector2f& pos) { position_ = pos; }
    
    // 速度访问
    const RenderCore::Vector2f& GetVelocity() const { return velocity_; }
    void SetVelocity(const RenderCore::Vector2f& vel) { velocity_ = vel; }
    
    // 旋转
    float GetRotation() const { return rotation_; }
    void SetRotation(float rot) { rotation_ = rot; }
};

} // namespace Game

#endif // GAME_OBJECT_H