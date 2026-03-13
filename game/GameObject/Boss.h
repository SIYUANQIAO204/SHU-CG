//
// Created by qiao on 25-11-19.
//

#ifndef PAINTENGINE_BOSS_H
#define PAINTENGINE_BOSS_H
#include "Enemy.h"
#include <vector>
namespace Game{
    struct BossPhase {
        float phaseStartHealth;           // 进入该阶段的血量阈值（或其他条件）
        BulletPattern pattern;            // 本阶段对应的弹幕配置
        float fireInterval;               // 本阶段的发射频率
        std::vector<Trajectory> trajectories;   // 轨迹数组
        float trajectorySwitchInterval = 3.0f;  // 轨迹切换时间
    };

    class Boss : public Enemy {
    private:
        std::vector<BossPhase> m_phases;  // 多个阶段
        int m_currentPhase = -1;          // 当前阶段下标
        int m_currentTrajectoryIndex = 0;       // 当前轨迹索引
        float m_trajectorySwitchTimer = 0.0f;   // 轨迹切换计时

        // 入场动画相关
        float m_entryTimer = 2.0f;              // 2秒入场时间
        bool m_isInvincible = true;
        RenderCore::Vector2f m_entryStartPos;   // 入场起始位置（屏幕外）
    public:
        Boss() = default;

        Boss(const RenderCore::Vector2f& pos, ObjectPool<Bullet>& bulletPool)
                : Enemy(pos, bulletPool) {}

        // 初始化 Boss 的阶段
        void Initialize(const RenderCore::Vector2f& pos,
                        const std::vector<BossPhase>& phases,
                        ObjectPool<Bullet>& bulletPool,
                        RenderCore::Vector2f* playerPos)
        {
            m_phases = phases;
            Enemy::Initialize(pos, phases[0].pattern, bulletPool, playerPos);
            m_currentPhase = 0;
            if (m_spawner) {
                m_spawner->SetPlayerPos(playerPos);
            }
            

            m_entryTimer = 2.0f;
            m_isInvincible = true;
            m_entryStartPos = {pos[0], -100.0f}; // 从屏幕上方100像素外入场
            
            // 初始位置设为屏幕外，等待Update中处理入场动画
            position_ = m_entryStartPos;
        }

        // 新增：设置各阶段轨迹
        /*void SetPhaseTrajectories(const std::vector<Trajectory>& trajectories) {
            // 确保轨迹数量与阶段数量匹配
            if (trajectories.size() == m_phases.size()) {
                for (size_t i = 0; i < m_phases.size(); ++i) {
                    m_phases[i].trajectory = trajectories[i];
                }
            }
        }*/

        void Update(float deltaTime) override;
        void SwitchPhase(int newPhase);

        float GetHealth() const { return m_health; }
        RenderCore::Rectangle GetBounds() const override; // 需要实现
        void Render(RenderCore::RenderEngine& engine) override; // 需要实现


        void SwitchTrajectory(int index);
        

        void TakeDamage(float damage) {
            if (m_isInvincible) return; // 无敌状态不受伤害
            
            m_health -= damage;
            if (m_health <= 0) {
                SetActive(false);
            }
        }
    };
}

#endif //PAINTENGINE_BOSS_H
