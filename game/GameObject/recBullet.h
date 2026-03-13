#ifndef REC_BULLET_H
#define REC_BULLET_H

#include "GameObject.h"

namespace Game {

class recBullet : public GameObject {
private:
    int damage_ = 1;
    bool isPlayerBullet_ = false;
    int size_ = 4; // 子弹大小
    
public:
    recBullet() : recBullet({0, 0}, {0, 0}, false) {}

    recBullet(const RenderCore::Vector2f& pos, const RenderCore::Vector2f& vel,
              bool isPlayerBullet = false)
        : GameObject(pos), isPlayerBullet_(isPlayerBullet) {
        SetVelocity(vel);
    }
    
    void Update(float deltaTime) override;
    void Render(RenderCore::RenderEngine& engine) override;
    RenderCore::Rectangle GetBounds() const override;
    
    // 获取碰撞半径
    float GetRadius() const { return size_ / 2.0f; };

    bool Getisplayerbullet() const {return isPlayerBullet_; };
};

} // namespace Game

#endif // REC_BULLET_H