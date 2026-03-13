// Enemy.cpp
#include "Enemy.h"
#include "engine.h"
#include "polygon.h"
#include "color.h"
#include "utils.h"

namespace Game {

Enemy::Enemy(const RenderCore::Vector2f& pos, ObjectPool<Bullet>& bulletPool)
    : GameObject(pos), m_spawner(std::make_unique<BulletSpawner>(bulletPool)) {
    // 配置圆形弹幕模式
    BulletPattern pattern;
    pattern.type = BulletPattern::Type::CIRCULAR;
    pattern.count = 8;
    pattern.speed = 2.0f;
    pattern.angle = 0.0f;
    m_spawner->SetPattern(pattern);
    
    // 向下移动
    SetVelocity({0.0f, 1.5f});
}

// 删除原有的构造函数，改为 Initialize 函数
void Enemy::Initialize(const RenderCore::Vector2f& pos, 
                       const BulletPattern& pattern,
                       ObjectPool<Bullet>& bulletPool,
                       RenderCore::Vector2f* playerPos) {
    position_ = pos;
    m_pattern = pattern; // 保存配置
    m_playerPos = playerPos; // 保存玩家位置
    m_spawner = std::make_unique<BulletSpawner>(bulletPool);
    m_spawner->SetPattern(pattern);
    SetVelocity({0.0f, 1.5f}); // 向下移动
    m_health = 100.0f; // 重置血量
}

void Enemy::Update(float deltaTime) {
    if (!IsActive()) return;
    
    // 如果使用轨迹移动，则忽略速度
    if (m_usingTrajectory) {
        position_ = m_trajectoryController.Update(deltaTime);

        m_spawner->SetPosition(position_);
    } else {
        // 传统的速度移动
        position_ += velocity_;
        m_spawner->SetPosition(position_);
    }
    
    // 出界销毁
    if (position_[1] > 620) {
        SetActive(false);
        return;
    }
    
    // 发射弹幕
    m_fireTimer += deltaTime;
    m_spawner->SetPosition(position_+ velocity_);
    m_spawner->Update(deltaTime);
    if (m_fireTimer >= m_fireInterval) {
        m_fireTimer = 0.0f;
        // 传递玩家位置给追踪导弹
        if (m_playerPos && m_pattern.type == BulletPattern::Type::HOMING) {
            m_spawner->Fire(*m_playerPos); // 指向玩家
        } else {
            m_spawner->Fire(); // 其他模式不指定目标
        }
    }
}

void Enemy::Render(RenderCore::RenderEngine& engine) {
    if (!IsActive() || position_[1] < -20 || position_[1] > 620) return;
    
    // 根据弹幕类型显示不同颜色（便于区分）
    RenderCore::Color renderColor = RenderCore::Colors::Red;
    switch (m_pattern.type) {
        case BulletPattern::Type::LINEAR: 
            renderColor = RenderCore::Colors::Red; 
            break;
        case BulletPattern::Type::CIRCULAR: 
            renderColor = RenderCore::Colors::Magenta; 
            break;
        case BulletPattern::Type::RADIAL: 
            renderColor = RenderCore::Colors::Yellow; 
            break;
        case BulletPattern::Type::SPIRAL: 
            renderColor = RenderCore::Colors::Cyan; 
            break;
        case BulletPattern::Type::HOMING: 
            renderColor = RenderCore::Colors::Green; 
            break;
    }
    
    RenderCore::Polygon triangle;
    triangle.emplace_back(static_cast<int>(position_[0]), 
                         static_cast<int>(position_[1] + 12));
    triangle.emplace_back(static_cast<int>(position_[0] - 8), 
                         static_cast<int>(position_[1] - 8));
    triangle.emplace_back(static_cast<int>(position_[0] + 8), 
                         static_cast<int>(position_[1] - 8));
    
    engine.set_pen_options({
        .color = renderColor,
        .fill_color = renderColor
    });
    engine.add_primitive(triangle);
}

RenderCore::Rectangle Enemy::GetBounds() const {
    int x = static_cast<int>(position_[0]);
    int y = static_cast<int>(position_[1]);
    return {{x - 8, y - 8}, {x + 8, y + 12}};
}

void Enemy::TakeDamage(float damage) {
    m_health -= damage;
    if (m_health <= 0) {
        SetActive(false);
    }
}

} // namespace Game