//
// Created by qiao on 25-11-19.
//

#pragma once

#include "GameObject.h"

#include <cmath>

namespace Game {

    class Laser : public GameObject {
    public:
        RenderCore::Vector2f m_startPoint;
        RenderCore::Vector2f m_direction; // 单位向量
        float m_length = 0.0f;            // 激光长度
        float m_width = 0.0f;             // 激光宽度
        int damage_ = 1;
        bool isPlayerBullet_ = false;
        float m_duration = 0.0f;          // 总持续时间
        float m_elapsed = 0.0f;           // 已经经过的时间
        float m_angleOffset = 0.0f;
        RenderCore::Vector2f* m_playerPos = nullptr;
    public:
        Laser(const RenderCore::Vector2f& start,
              const RenderCore::Vector2f& direction,
              float length,
              float width,
              float duration,
              RenderCore::Vector2f* playerPos = nullptr);

        void Update(float deltaTime) override;
        void Render(RenderCore::RenderEngine& engine) override;
        RenderCore::Rectangle GetBounds() const override;
        float GetRadius() const { return m_width / 2.0f; };
        bool Getisplayerbullet() const { return  isPlayerBullet_;};
        const RenderCore::Vector2f& GetStartPoint() const { return m_startPoint; }
        void SetTarget(RenderCore::Vector2f* targetPos) { m_playerPos = targetPos; }
        void UpdateDirection() {
            if (!m_playerPos) return;
            
            RenderCore::Vector2f toTarget = *m_playerPos - m_startPoint;
            float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
            if (len > 0.0001f) {
                float baseAngle = std::atan2(toTarget.y, toTarget.x);
                // 应用偏移角度
                float finalAngle = baseAngle + m_angleOffset;
                m_direction = {std::cos(finalAngle), std::sin(finalAngle)};
            }
        }
    };

} // namespace Game



