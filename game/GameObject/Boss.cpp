//
// Created by qiao on 25-11-19.
//

#include "Boss.h"

namespace Game {
    void Boss::Update(float deltaTime)
    {

        if (m_entryTimer > 0.0f) {
            m_entryTimer -= deltaTime;
            float t = 1.0f - (m_entryTimer / 2.0f); // 插值系数
            
            // 从屏幕上方(-100)平滑移动到第一阶段第一个轨迹起点
            position_ = m_entryStartPos + (m_phases[0].trajectories[0].start - m_entryStartPos) * t;
            
            if (m_entryTimer <= 0.0f) {
                m_isInvincible = false;                 // 结束无敌
                SwitchTrajectory(0);                    // 开始第一段轨迹
            }
            return; // 入场期间不执行其他逻辑
        }
        
        // 1. 调用父类 Enemy::Update 处理子弹发射
        Enemy::Update(deltaTime);

        // 确保激光发射时传递玩家位置（用于追踪）
        if (m_spawner && m_playerPos) {
        m_spawner->SetPosition(position_);
        m_spawner->Update(deltaTime);
        
        // 只在激光模式下传递玩家位置
        if (m_fireTimer >= m_fireInterval && m_pattern.type == BulletPattern::Type::LASER) {
            m_fireTimer = 0.0f;
            m_spawner->Fire(*m_playerPos);
        }
    }
        

        if (!m_phases.empty() && m_currentPhase >= 0) {
            auto& phase = m_phases[m_currentPhase];
            if (!phase.trajectories.empty()) {
                m_trajectorySwitchTimer += deltaTime;
                if (m_trajectorySwitchTimer >= phase.trajectorySwitchInterval) {
                    m_trajectorySwitchTimer = 0.0f;
                    m_currentTrajectoryIndex = (m_currentTrajectoryIndex + 1) % phase.trajectories.size();
                    SwitchTrajectory(m_currentTrajectoryIndex);
                }
            }
        }
        
        // 3. 血量检测与阶段切换
        if (m_currentPhase + 1 < (int)m_phases.size()) {
            float nextHealth = m_phases[m_currentPhase + 1].phaseStartHealth;
            if (m_health <= nextHealth) {
                SwitchPhase(m_currentPhase + 1);
            }
        }
    }

    void Boss::SwitchPhase(int newPhase)
    {
        m_currentPhase = newPhase;
        auto& phase = m_phases[newPhase];
        
        // 切换弹幕配置
        m_pattern = phase.pattern;
        m_fireInterval = phase.fireInterval;
        if (m_spawner) {
            m_spawner->SetPattern(phase.pattern);
        }
        

        m_currentTrajectoryIndex = 0;
        m_trajectorySwitchTimer = 0.0f;
        SwitchTrajectory(0);
        
        std::cout << "Boss切换至阶段 " << newPhase + 1 << std::endl;
    }

    //新增：切换到指定轨迹
    void Boss::SwitchTrajectory(int index) {
        if (m_phases.empty() || m_currentPhase < 0) return;
        
        auto& phase = m_phases[m_currentPhase];
        if (index >= (int)phase.trajectories.size()) return;
        
        // 复用Enemy的SetTrajectory，速度0.05f适合Boss
        SetTrajectory(phase.trajectories[index], 0.25f);
        m_trajectoryController.Reset();
        m_usingTrajectory = true;
        
        std::cout << "  → 切换轨迹 " << index + 1 << "/" << phase.trajectories.size() << std::endl;
    }

    void Boss::Render(RenderCore::RenderEngine& engine) {
        if (!IsActive()) return;
        
        // 根据阶段改变颜色
        RenderCore::Color colors[] = {
            RenderCore::Colors::Magenta,
            RenderCore::Colors::Cyan,
            RenderCore::Colors::Red,
            RenderCore::Colors::Yellow
        };
        
        RenderCore::Color bossColor = colors[std::min(m_currentPhase, 3)];
        
        // 绘制大型三角形作为Boss
        RenderCore::Polygon triangle;
        triangle.emplace_back(static_cast<int>(position_[0]), 
                            static_cast<int>(position_[1] + 20));
        triangle.emplace_back(static_cast<int>(position_[0] - 20), 
                            static_cast<int>(position_[1] - 20));
        triangle.emplace_back(static_cast<int>(position_[0] + 20), 
                            static_cast<int>(position_[1] - 20));
        
        engine.set_pen_options({
            .color = bossColor,
            .fill_color = bossColor
        });
        engine.add_primitive(triangle);
        
        // 绘制血条背景
        engine.set_pen_options({
            .color = RenderCore::Colors::DarkGray,
            .fill_color = RenderCore::Colors::DarkGray
        });
        engine.add_primitive(RenderCore::make_rectangle(
            {static_cast<int>(position_[0] - 30), static_cast<int>(position_[1] - 30)},
            {static_cast<int>(position_[0] + 30), static_cast<int>(position_[1] - 25)}
        ));
        
        // 绘制当前血量
        float healthPercent = m_health / 100.0f;
        engine.set_pen_options({
            .color = RenderCore::Colors::Green,
            .fill_color = RenderCore::Colors::Green
        });
        engine.add_primitive(RenderCore::make_rectangle(
            {static_cast<int>(position_[0] - 30), static_cast<int>(position_[1] - 30)},
            {static_cast<int>(position_[0] - 30 + static_cast<int>(60 * healthPercent)), 
            static_cast<int>(position_[1] - 25)}
        ));
    }

    RenderCore::Rectangle Boss::GetBounds() const {
        int x = static_cast<int>(position_[0]);
        int y = static_cast<int>(position_[1]);
        return {{x - 20, y - 20}, {x + 20, y + 20}};
    }

} // namespace Game
