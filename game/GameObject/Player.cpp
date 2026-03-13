#include "Player.h"
#include "engine.h"
#include "polygon.h"
#include "color.h"
#include "utils.h"

namespace Game {

void Player::Update(float deltaTime) {
    // 输入处理在 GameScene 中统一进行
    // 根据速度更新位置
    position_ += velocity_;
    
    // 边界检查：限制在屏幕范围内
    // 自机三角形大小约为 16x20 像素
    const float halfWidth = 8.0f;   // 三角形半宽
    const float halfHeight = 10.0f; // 三角形半高
    
    // 使用引擎的数学函数进行裁剪
    position_[0] = RenderCore::max(halfWidth, 
                                   RenderCore::min(800.0f - halfWidth, position_[0]));
    position_[1] = RenderCore::max(halfHeight, 
                                   RenderCore::min(600.0f - halfHeight, position_[1]));

    // 无敌计时递减
    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= deltaTime;
        m_flashTimer += deltaTime;
        if (m_invincibleTimer <= 0.f) {
            m_isInvincible = false;
            m_flashTimer = 0.f;
        }
    }
}

void Player::Render(RenderCore::RenderEngine& engine) {
    bool flashVisible = true;
    // 处于无敌闪烁时，每0.2s消失/出现
    if (m_isInvincible) {
        if (static_cast<int>(m_flashTimer * 5) % 2 == 0) { // 闪烁频率5Hz即0.2s
            flashVisible = true;
        } else {
            flashVisible = false;
        }
    }

    if (flashVisible) {
        // 普通渲染
        RenderCore::Polygon triangle;
        triangle.emplace_back(static_cast<int>(position_[0]), static_cast<int>(position_[1] - 12));
        triangle.emplace_back(static_cast<int>(position_[0] - 8), static_cast<int>(position_[1] + 8));
        triangle.emplace_back(static_cast<int>(position_[0] + 8), static_cast<int>(position_[1] + 8));

        engine.set_pen_options({
            .color = RenderCore::Colors::White,
            .fill_color = RenderCore::Colors::White
        });
        engine.add_primitive(triangle);

        // 碰撞点
        engine.set_pen_options({
            .color = RenderCore::Colors::Red,
            .fill_color = RenderCore::Colors::Red
        });
        engine.add_primitive(RenderCore::make_circle_center_radius(
            {static_cast<int>(position_[0]), static_cast<int>(position_[1])}, 2
        ));
    }
}

RenderCore::Rectangle Player::GetBounds() const {
    // 返回AABB包围盒
    int x = static_cast<int>(position_[0]);
    int y = static_cast<int>(position_[1]);
    return {{x - 8, y - 12}, {x + 8, y + 8}};
}

// 受伤调用，减少生命并进入无敌
void Player::TakeDamage() {
    if (m_isInvincible || m_lives <= 0) return;
    m_lives--;
    m_isInvincible = true;
    m_invincibleTimer = 2.0f;
    m_flashTimer = 0.f;
}

void Player::AddLife(int count) {
    m_lives += count;
    if (m_lives > m_maxLives) m_lives = m_maxLives;
}

// 外部直接启动无敌
void Player::SetInvincible(float timer) {
    m_isInvincible = true;
    m_invincibleTimer = timer;
    m_flashTimer = 0.f;
}

} // namespace Game