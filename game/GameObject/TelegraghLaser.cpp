#include "TelegraghLaser.h"

namespace Game {

    TelegraphLaser::TelegraphLaser(const RenderCore::Vector2f& start,
                                   const RenderCore::Vector2f& direction,
                                   float length,
                                   float width,
                                   float warningDuration,
                                   float fireDuration,
                                   RenderCore::Vector2f* playerPos,
                                   float angleOffset)
            : Laser(start, direction, length, width, warningDuration + fireDuration,playerPos),
              m_warningDuration(warningDuration),
              m_fireDuration(fireDuration)
    {
        m_state = State::Warning;
        m_angleOffset = angleOffset;
    }

    void TelegraphLaser::Update(float deltaTime)
    {
        if (!active_) return;

        m_elapsed += deltaTime;

        // 在预警过半时锁定角度
        if (m_state == State::Warning && !m_isLocked && m_elapsed >= m_warningDuration * 0.5f) {
            m_isLocked = true;
            m_lockedDirection = m_direction; // 记录当前方向
            std::cout << "激光角度已锁定！" << std::endl;
        }

        // 未锁定且发射阶段才追踪
        if (!m_isLocked && m_playerPos) {
            UpdateDirection(); // 持续追踪
        }
        else if (m_isLocked) {
            m_direction = m_lockedDirection; // 使用锁定方向
        }

        switch (m_state) {
            case State::Warning:
                if (m_elapsed >= m_warningDuration) {
                    m_state = State::Firing;
                    m_elapsed = 0.0f;
                }
                break;

            case State::Firing:
                if (m_elapsed >= m_fireDuration) {
                    m_state = State::Finished;
                    active_ = false;
                    m_isLocked = false; // 重置锁定状态
                }
                break;

            default:
                break;
        }
    }

    void TelegraphLaser::Render(RenderCore::RenderEngine& engine)
    {
        if (!active_) return;

        RenderCore::Vector2f endPoint {
                m_startPoint.x + m_direction.x * m_length,
                m_startPoint.y + m_direction.y * m_length
        };

        switch (m_state) {
            case State::Warning:
                // 预警阶段：细线 + 半透明蓝色
                engine.set_pen_options({
                    .color = isPlayerBullet_ ? RenderCore::Colors::Blue : RenderCore::Colors::Red,
                    .width = 1  // 细线预警
                });
                break;

            case State::Firing:
                // 发射阶段：粗线 + 高亮黄色
                engine.set_pen_options({
                    .color = isPlayerBullet_ ? RenderCore::Colors::Cyan : RenderCore::Colors::Yellow,
                    .width = static_cast<int>(m_width)  // 粗线发射
                });
                break;

            default:
                return;
        }

        // 统一渲染为线段
        engine.add_primitive(RenderCore::make_line(
            {static_cast<int>(m_startPoint.x), static_cast<int>(m_startPoint.y)},
            {static_cast<int>(endPoint.x), static_cast<int>(endPoint.y)},
            RenderCore::Line::LineAlgorithm::BRESENHAM
        ));
    }

    RenderCore::Rectangle TelegraphLaser::GetWaringBounds() const {
        // 警告阶段的精确AABB（细线）
        RenderCore::Vector2f endPoint{
                m_startPoint.x + m_direction.x * m_length,
                m_startPoint.y + m_direction.y * m_length
        };

        float minX = std::min(m_startPoint.x, endPoint.x) - 0.5f;
        float maxX = std::max(m_startPoint.x, endPoint.x) + 0.5f;
        float minY = std::min(m_startPoint.y, endPoint.y) - 0.5f;
        float maxY = std::max(m_startPoint.y, endPoint.y) + 0.5f;

        return {{static_cast<int>(minX), static_cast<int>(minY)},
                {static_cast<int>(maxX), static_cast<int>(maxY)}};
    }
}