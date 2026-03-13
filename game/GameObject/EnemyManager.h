// EnemyManager.h
#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include "Enemy.h"
#include "TrajectoryController.h"
#include "../Utils/ObjectPool.h"
#include <ctime>

namespace Game {

class EnemyManager {
public:
    enum class Difficulty {
        EASY,
        NORMAL,
        HARD
    };
private:
    ObjectPool<Enemy> m_enemyPool;
    float m_spawnTimer = 0.0f;
    float m_spawnInterval = 5.0f; // 每5秒生成
    ObjectPool<Bullet>& m_bulletPool;
    bool m_spawningEnabled = true;  // 是否允许生成敌机
    Difficulty m_difficulty = Difficulty::NORMAL;

public:
    explicit EnemyManager(ObjectPool<Bullet>& bulletPool)
        : m_enemyPool(50), m_bulletPool(bulletPool) {srand(static_cast<unsigned>(time(nullptr)));}
    
    void Update(float deltaTime,RenderCore::Vector2f& playerPos);
    void Render(RenderCore::RenderEngine& engine);
    
    ObjectPool<Enemy>& GetPool() { return m_enemyPool; }
    void SpawnEnemy(const RenderCore::Vector2f& position,RenderCore::Vector2f& playerPos);
    void SetSpawningEnabled(bool enabled) { m_spawningEnabled = enabled; }

    // 新增难度接口
    void SetDifficulty(Difficulty diff) {
        m_difficulty = diff;
        switch(diff) {
            case Difficulty::EASY:   m_spawnInterval = 5.0f + static_cast<float>(rand() % 3); break;
            case Difficulty::NORMAL: m_spawnInterval = 3.0f + static_cast<float>(rand() % 3); break;
            case Difficulty::HARD:   m_spawnInterval = 2.0f + static_cast<float>(rand() % 3); break;
        }
    }

};

} // namespace Game

#endif