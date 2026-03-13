//
// Created by qiao on 25-11-19.
//

#include "Laser.h"
#include "Laser.h"
#include <algorithm>

namespace Game {

    Laser::Laser(const RenderCore::Vector2f &start,
                 const RenderCore::Vector2f &direction,
                 float length,
                 float width,
                 float duration,
                 RenderCore::Vector2f* playerPos) {
        m_startPoint = start;
        m_direction = direction;
        m_playerPos = playerPos;

        // 归一化方向向量
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0.0001f) {
            m_direction.x /= len;
            m_direction.y /= len;
        }

        m_length = length;
        m_width = width;
        m_duration = duration;

        // 激光不需要 velocity，但你可以给它速度（例如移动光束）
        position_ = start;
    }

    void Laser::Update(float deltaTime) {
        if (!active_) return;

        m_elapsed += deltaTime;

        if (m_playerPos && m_duration > 0.0f) {
            RenderCore::Vector2f toPlayer = *m_playerPos - m_startPoint;
            float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
            if (len > 0.0001f) {
                m_direction = {toPlayer.x / len, toPlayer.y / len}; // 每帧更新方向
            }
        }
        
        if (m_elapsed >= m_duration) {
            active_ = false;
        }
    }

    void Laser::Render(RenderCore::RenderEngine &engine) {
        if (!active_) return;

        // 可视化方式取决于你的 RenderEngine
        // 我用一个简单的 draw_line + width 举例
        RenderCore::Vector2f endPoint{
                m_startPoint.x + m_direction.x * m_length,
                m_startPoint.y + m_direction.y * m_length
        };
        RenderCore::Rectangle rect = GetBounds();
        // 渲染为粗直线：设置线宽和颜色
        engine.set_pen_options({
            .color = isPlayerBullet_ ? RenderCore::Colors::Cyan : RenderCore::Colors::Yellow,
            .width = static_cast<int>(m_width)  // 关键：控制线宽
        });
        
        // 使用Line图元而非Rectangle
        engine.add_primitive(RenderCore::make_line(
            {static_cast<int>(m_startPoint.x), static_cast<int>(m_startPoint.y)},
            {static_cast<int>(endPoint.x), static_cast<int>(endPoint.y)},
            RenderCore::Line::LineAlgorithm::BRESENHAM
        ));

    }

    RenderCore::Rectangle Laser::GetBounds() const {
        // 用 AABB 简化（包围整个激光）
        RenderCore::Vector2f endPoint{
                m_startPoint.x + m_direction.x * m_length,
                m_startPoint.y + m_direction.y * m_length
        };

        // 返回精确的线段AABB包围盒（带线宽偏移）
        float halfWidth = m_width / 2.0f;
        float minX = std::min(m_startPoint.x, endPoint.x) - halfWidth;
        float maxX = std::max(m_startPoint.x, endPoint.x) + halfWidth;
        float minY = std::min(m_startPoint.y, endPoint.y) - halfWidth;
        float maxY = std::max(m_startPoint.y, endPoint.y) + halfWidth;

        return {{static_cast<int>(minX), static_cast<int>(minY)},
                {static_cast<int>(maxX), static_cast<int>(maxY)}};
    };
}