// EnemyManager.cpp
#define _USE_MATH_DEFINES
#include "EnemyManager.h"
#include <cstdlib>
#include <math.h>

namespace Game {

// 静态函数，生成随机弹幕
static BulletPattern GenerateRandomPattern() {
    BulletPattern pattern;
    // 随机类型 (0-4)
    int type = rand() % 5; 
    pattern.type = static_cast<BulletPattern::Type>(type);
    
    // 基础参数
    pattern.speed = 2.0f + static_cast<float>(rand() % 3); // 2-4
    pattern.angle = static_cast<float>(rand() % 360) * (M_PI / 180.0f); // 0-360度
    
    // 类型特有参数
    switch (pattern.type) {
        case BulletPattern::Type::LINEAR:
            pattern.count = 1;
            break;
        case BulletPattern::Type::CIRCULAR:
            pattern.count = 6 + rand() % 6; // 6-11发
            break;
        case BulletPattern::Type::RADIAL:
            pattern.count = 3 + rand() % 5; // 3-7发
            pattern.angleSpread = (30.0f + rand() % 60) * (M_PI / 180.0f); // 30-90度
            break;
        case BulletPattern::Type::SPIRAL:
            pattern.spiralSpeed = 180.0f + rand() % 180;
            pattern.spiralRadius = 0; 
            break;
        case BulletPattern::Type::HOMING:
            pattern.count = 1;
            break;
    }
    
    return pattern;
}

void EnemyManager::Update(float deltaTime,RenderCore::Vector2f& playerPos) {
    if (!m_spawningEnabled) return;  // 禁止生成时直接返回

    m_spawnTimer += deltaTime;

    float interval = m_spawnInterval;
    if (m_spawnTimer >= interval) {
        m_spawnTimer = 0.0f;

        int enemyCount = 1 ;
        for (int i = 0; i < enemyCount; ++i) {
            // 随机X位置
            float x = 100.0f + static_cast<float>(rand() % 600);
            // 随机延迟出现
            float delay = static_cast<float>(i) * 0.5f;
            
            // 使用定时器或直接在循环中生成（简化）
            SpawnEnemy({x, -50.0f - static_cast<float>(i * 50)}, playerPos);
        }
    }
    
    m_enemyPool.ForEachActive([deltaTime](Enemy& enemy) {
        enemy.Update(deltaTime);
    });
}

void EnemyManager::Render(RenderCore::RenderEngine& engine) {
    m_enemyPool.ForEachActive([&engine](Enemy& enemy) {
        enemy.Render(engine);
    });
}

void EnemyManager::SpawnEnemy(const RenderCore::Vector2f& position,RenderCore::Vector2f& playerPos) {
    // 生成随机弹幕配置
    BulletPattern pattern = GenerateRandomPattern();
    
    // 从对象池获取敌机并初始化
    Enemy* enemy = m_enemyPool.Acquire(); // 调用默认构造函数
    enemy->Initialize(position, pattern, m_bulletPool,&playerPos);
    enemy->SetActive(true);

    // 设置随机轨迹
    TrajectoryType trajType = TrajectoryController::RandomType();
    RenderCore::Vector2f endPos;

    // 随机生成终点位置（屏幕底部附近）
    endPos[0] = 100.0f + static_cast<float>(rand() % 600);
    endPos[1] = 500.0f + static_cast<float>(rand() % 100);

    Trajectory traj;
    traj.type = trajType;
    traj.start = position;
    traj.end = endPos;

    switch (trajType) {
        case TrajectoryType::LINEAR:
            // 直线轨迹不需要额外参数
            enemy->SetTrajectory(traj, 0.08f); // 速度参数
            break;

        case TrajectoryType::ARC: {
            // 随机生成圆心（在起点和终点之间）
            RenderCore::Vector2f center;
            center[0] = (position[0] + endPos[0]) / 2 + (rand() % 200 - 100); // 左右偏移
            center[1] = (position[1] + endPos[1]) / 2 - 100; // 向上偏移
            traj.arcCenter = center;
            enemy->SetTrajectory(traj, 0.06f);
            break;
        }

        case TrajectoryType::BEZIER: {
            // 添加两个控制点
            std::vector<RenderCore::Vector2f> controlPoints;
            RenderCore::Vector2f cp1, cp2;

            // 第一个控制点
            cp1[0] = position[0] + (rand() % 200 - 100);
            cp1[1] = position[1] + 100 + (rand() % 100);

            // 第二个控制点
            cp2[0] = endPos[0] + (rand() % 200 - 100);
            cp2[1] = endPos[1] - 100 + (rand() % 100);

            controlPoints.push_back(cp1);
            controlPoints.push_back(cp2);
            traj.controlPoints = controlPoints;

            enemy->SetTrajectory(traj, 0.07f);
            break;
        }
    }

    // 控制台输出（调试用）
    std::cout << "Spawned Enemy with pattern type: " 
              << static_cast<int>(pattern.type)
              << " and trajectory type: " << static_cast<int>(trajType)<< std::endl;
}

} // namespace Game