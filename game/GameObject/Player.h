#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"

namespace Game {

class Player : public GameObject {
private:
    float speed_ = 5.0f;          // 移动速度
    int hitRadius_ = 8;           // 碰撞半径

    // 玩家扩展：血量与无敌
    int m_lives = 3;              // 当前剩余生命
    int m_maxLives = 3;           // 最大生命（可由难度调整）
    float m_invincibleTimer = 0.f;// 无敌剩余时间
    bool m_isInvincible = false;  // 无敌标记
    float m_flashTimer = 0.f;     // 闪烁计时（视觉提示）
    
public:
    Player() : GameObject({400, 500}) {} // 初始位置在屏幕下方
    Player(const RenderCore::Vector2f& pos) : GameObject(pos) {}
    
    void Update(float deltaTime) override;
    void Render(RenderCore::RenderEngine& engine) override;
    RenderCore::Rectangle GetBounds() const override;

    // 血量相关
    void TakeDamage();        // 被击中扣1，无敌2秒
    void AddLife(int count = 1); // 增加生命
    int GetLives() const { return m_lives; }
    int GetMaxLives() const { return m_maxLives; }
    bool IsInvincible() const { return m_isInvincible; }
    void SetInitialLives(int cnt) { m_lives = m_maxLives = cnt; }
    float GetInvincibleTimer() const { return m_invincibleTimer; } // 渲染用
    void SetInvincible(float timer = 2.0f);
    
    // 获取碰撞圆形（用于精确检测）
    RenderCore::Vector2f GetHitCenter() const { 
        return {position_[0], position_[1]}; 
    }
    float GetHitRadius() const { return hitRadius_; }
};

} // namespace Game

#endif // PLAYER_H