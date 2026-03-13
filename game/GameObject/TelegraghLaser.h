//
// Created by qiao on 25-11-19.
//

#pragma once

#include "Laser.h"

namespace Game {

    class TelegraphLaser : public Laser {
    public:
        float m_warningDuration;     // 预警时间
        float m_fireDuration;        // 发射时间
        float m_elapsed = 0.0f;
        bool m_isLocked = false;
        RenderCore::Vector2f m_lockedDirection;

        enum class State {
            Warning,
            Firing,
            Finished
        } m_state = State::Warning;

    public:
        TelegraphLaser(const RenderCore::Vector2f& start,
                       const RenderCore::Vector2f& direction,
                       float length,
                       float width,
                       float warningDuration,
                       float fireDuration,
                       RenderCore::Vector2f* playerPos = nullptr,
                       float angleOffset = 0.0f);

        void Update(float deltaTime) override;
        void Render(RenderCore::RenderEngine& engine) override;
        bool Getisplayerbullet() const {return isPlayerBullet_;};
        // firing 阶段才有伤害
        bool IsDangerous() const { return m_state == State::Firing; }
        RenderCore::Rectangle GetWaringBounds() const ;
    };

} // namespace Game

