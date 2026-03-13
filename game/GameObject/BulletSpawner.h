#ifndef BULLET_SPAWNER_H
#define BULLET_SPAWNER_H

#include "recBullet.h"
#include "Bullet.h"
#include "../Utils/ObjectPool.h"
#include "vector.h"
#include <functional>

namespace Game {

// 子弹发射配置（为后续弹幕模式扩展）
struct BulletPattern {
    enum class Type {
        LINEAR,      // 直线
        CIRCULAR,    // 圆形扩散
        RADIAL,      // 辐射状
        SPIRAL,      // 螺旋
        HOMING,       // 追踪
        LASER        //新增：预警激光
    } type = Type::LINEAR;
    
    int count = 1;              // 子弹数量
    float speed = 3.0f;         // 速度
    float angle = 0.0f;         // 发射角度
    float angleSpread = 0.0f;   // 角度扩散（用于扇形）
    float angleDelta = 0.0f;    // 每帧角度增量（用于旋转）

    float spiralSpeed = 5.0f;   // 螺旋角速度（度/秒）
    float spiralRadius = 0.0f;  // 螺旋半径（0表示从中心发出）

    // 激光专用参数
    float laserLength = 150.0f;     // 激光长度
    float laserWidth = 8.0f;        // 激光宽度
    float warningDuration = 1.0f;   // 预警时间
    float fireDuration = 0.5f;      // 发射时间
    int laserCount = 1;           // 新增：激光数量
    float laserAngleSpread = 0.0f; //新增：激光角度扩散（扇形）
};

// 子弹发射器（可挂载到Enemy或Boss）
class BulletSpawner {
private:
    ObjectPool<Bullet>& bulletPool_;  // 引用对象池
    BulletPattern pattern_;
    float timer_ = 0.0f;
    float m_spiralAngle = 0.0f; // 螺旋当前角度

    // 当前发射位置（解决position未定义问题）
    RenderCore::Vector2f currentPosition_ = {0, 0};
    RenderCore::Vector2f* m_playerPos = nullptr;
    
public:
    explicit BulletSpawner(ObjectPool<Bullet>& pool) : bulletPool_(pool) {}
    
    // 设置发射模式
    void SetPattern(const BulletPattern& pattern) { pattern_ = pattern; }

    // 每帧设置发射源位置（如Boss或敌人位置）
    void SetPosition(const RenderCore::Vector2f& pos) { currentPosition_ = pos; }

    // 设置玩家位置
    void SetPlayerPos(RenderCore::Vector2f* pos) { m_playerPos = pos; }
    
    // 发射子弹（灵活接口，支持未来扩展）
    void Fire(const RenderCore::Vector2f& target = {0, 0});

    // 重载：允许临时指定位置发射
    void FireAt(const RenderCore::Vector2f& position, const RenderCore::Vector2f& target = {0, 0});

    void FireLaser(const RenderCore::Vector2f &pos,
                   const RenderCore::Vector2f &dir);

    // 更新（处理角度增量等）
    void Update(float deltaTime);
    
private:
    // 发射单发子弹
    void FireSingle(const RenderCore::Vector2f& pos, const RenderCore::Vector2f& vel);
};

// 全局子弹管理器（场景持有）
class BulletManager {
private:
    ObjectPool<Bullet> bulletPool_{200};  // 初始200发
    BulletSpawner spawner_;
    
public:
    BulletManager() : spawner_(bulletPool_) {}
    
    // 获取对象池引用
    ObjectPool<Bullet>& GetPool() { return bulletPool_; }

    // 新增：每帧设置发射器位置
    void SetSpawnerPosition(const RenderCore::Vector2f& pos) { spawner_.SetPosition(pos); }
    
    // 发射接口（简化版）
    void Spawn(const RenderCore::Vector2f& pos, const RenderCore::Vector2f& vel, bool isPlayerBullet = false) {
        Bullet* bullet = bulletPool_.Acquire(pos, vel, isPlayerBullet);
        bullet->SetActive(true);
    }
    
    // 获取发射器（用于复杂弹幕）
    BulletSpawner& GetSpawner() { return spawner_; }
    
    // 更新所有子弹
    void Update(float deltaTime) {
        // 1. 更新发射器（角度增量等）
        spawner_.Update(deltaTime);
        
        // 2. 更新所有激活的子弹
        bulletPool_.ForEachActive([deltaTime](Bullet& bullet) {
            bullet.Update(deltaTime);
        });
        
        // 3. 清理标记为非激活的子弹（出界或碰撞）
        bulletPool_.CleanupInactive();
    }
    
    // 渲染所有子弹
    void Render(RenderCore::RenderEngine& engine) {
        bulletPool_.ForEachActive([&engine](Bullet& bullet) {
            bullet.Render(engine);
        });
    }
};

} // namespace Game

#endif // BULLET_SPAWNER_H