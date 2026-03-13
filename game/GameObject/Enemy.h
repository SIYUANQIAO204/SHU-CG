// Enemy.h
#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include "BulletSpawner.h"
#include "TrajectoryController.h"
#include "../Utils/ObjectPool.h"
#include <memory>

namespace Game {

class Enemy : public GameObject {
protected:
    std::unique_ptr<BulletSpawner> m_spawner;
    float m_fireTimer = 0.0f;
    float m_fireInterval = 1.0f; // 每秒发射一次
    BulletPattern m_pattern; // 存储自己的弹幕配置
    RenderCore::Vector2f* m_playerPos = nullptr; // 玩家位置指针
    float m_health = 100.0f;

    TrajectoryController m_trajectoryController;  // 轨迹控制器
    bool m_usingTrajectory = false;               // 是否使用轨迹移动

public:
    Enemy() : GameObject({0, 0}) {}
    Enemy(const RenderCore::Vector2f& pos, ObjectPool<Bullet>& bulletPool);

    // 初始化函数，接收位置和弹幕配置
    void Initialize(const RenderCore::Vector2f& pos, 
                    const BulletPattern& pattern, 
                    ObjectPool<Bullet>& bulletPool,
                    RenderCore::Vector2f* playerPos);

    // 设置轨迹
    void SetTrajectory(const Trajectory& trajectory, float speed = 0.1f) {
        m_trajectoryController.SetSpeed(speed);
        switch (trajectory.type) {
            case TrajectoryType::LINEAR:
                m_trajectoryController.SetLinearTrajectory(trajectory.start, trajectory.end);
                break;
            case TrajectoryType::ARC:
                m_trajectoryController.SetArcTrajectory(trajectory.start, trajectory.end, trajectory.arcCenter);
                break;
            case TrajectoryType::BEZIER:
                m_trajectoryController.SetBezierTrajectory(trajectory.start, trajectory.end, trajectory.controlPoints);
                break;
        }
        m_usingTrajectory = true;
    }

    // 停止使用轨迹
    void StopTrajectory() {
        m_usingTrajectory = false;
    }

    void Update(float deltaTime) override;
    void Render(RenderCore::RenderEngine& engine) override;
    RenderCore::Rectangle GetBounds() const override;
    
    void TakeDamage(float damage);
    bool IsDead() const { return m_health <= 0; }
    
    BulletSpawner* GetSpawner() { return m_spawner.get(); }
};

} // namespace Game

#endif