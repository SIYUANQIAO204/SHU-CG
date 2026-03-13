#define _USE_MATH_DEFINES
#include "BulletSpawner.h"
#include "vector.h"
#include <cmath>
#include <iostream>


namespace Game {

void BulletSpawner::Fire(const RenderCore::Vector2f& target) {
    FireAt(currentPosition_, target);
}

void BulletSpawner::FireAt(const RenderCore::Vector2f& position, const RenderCore::Vector2f& target) {
    currentPosition_ = position;

    switch (pattern_.type) {
        case BulletPattern::Type::LINEAR: {
            float baseAngle = M_PI / 2.0f;
            // 直线发射
            RenderCore::Vector2f velocity = {
                std::cos(baseAngle + pattern_.angle) * pattern_.speed,
                std::sin(baseAngle + pattern_.angle) * pattern_.speed
            };
            FireSingle(position, velocity);
            break;
        }
        
        case BulletPattern::Type::CIRCULAR: {
            float baseAngle = M_PI / 2.0f;
            // 圆形扩散
            float angleStep = (2.0f * M_PI) / pattern_.count;
            for (int i = 0; i < pattern_.count; ++i) {
                float angle = baseAngle + angleStep * i + pattern_.angle;
                RenderCore::Vector2f velocity = {
                    std::cos(angle) * pattern_.speed,
                    std::sin(angle) * pattern_.speed
                };
                FireSingle(position, velocity);
            }
            break;
        }
        
        case BulletPattern::Type::RADIAL: {
            float baseAngle = M_PI / 2.0f;
            // 扇形扩散
            float halfSpread = pattern_.angleSpread / 2.0f;
            float angleStep = pattern_.count > 1 ? pattern_.angleSpread / (pattern_.count - 1) : 0.0f;
            for (int i = 0; i < pattern_.count; ++i) {
                float angle = baseAngle + pattern_.angle - halfSpread + angleStep * i;
                RenderCore::Vector2f velocity = {
                    std::cos(angle) * pattern_.speed,
                    std::sin(angle) * pattern_.speed
                };
                FireSingle(position, velocity);
            }
            break;
        }
        
        case BulletPattern::Type::SPIRAL:{
            float baseAngle = M_PI / 2.0f;
            // 螺旋模式：在Update中累积角度，这里只发射单发
            RenderCore::Vector2f velocity = {
                std::cos(baseAngle + pattern_.angle) * pattern_.speed,
                std::sin(baseAngle + pattern_.angle) * pattern_.speed
            };
            FireSingle(position, velocity);
            break;
        }
            
        case BulletPattern::Type::HOMING: {
            // 追踪模式（需要目标位置）
            RenderCore::Vector2f dir = vector_normalize(target - position);
            RenderCore::Vector2f velocity = dir * pattern_.speed;
            FireSingle(position, velocity);
            break;
        }

        case BulletPattern::Type::LASER: {
            // 激光方向（默认向下，可扩展为瞄准玩家）
            RenderCore::Vector2f direction = {0.0f, 1.0f}; // 默认向下

            // 如果提供了有效目标（非零向量），则追踪目标
            if (target.x != 0.0f || target.y != 0.0f) {
                RenderCore::Vector2f toTarget = target - position;
                float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
                if (len > 0.0001f) {
                    direction = {toTarget.x / len, toTarget.y / len};
                }
            }
            
            FireLaser(position, direction);
            break;
        }
    }
}

void BulletSpawner::FireLaser(const RenderCore::Vector2f& pos, const RenderCore::Vector2f& dir) {
    // 从对象池获取Bullet，然后原位构造TelegraphLaser
    float baseAngle = std::atan2(dir.y, dir.x);
    float angleStep = pattern_.laserCount > 1 ? pattern_.laserAngleSpread / (pattern_.laserCount - 1) : 0.0f;
    
    for (int i = 0; i < pattern_.laserCount; ++i) {
        // 计算固定偏角（相对主激光）
        float fixedOffset = (pattern_.laserCount > 1) ? 
            -pattern_.laserAngleSpread / 2.0f + angleStep * i : 0.0f;
        
        // 为副激光添加随机偏移（±5度）
        float randomOffset = (i == 0) ? 0.0f : (rand() % 10 - 5) * (M_PI / 180.0f);
        float totalOffset = fixedOffset + randomOffset;
        
        // 所有激光都追踪玩家（包括带偏移的）
        Bullet* bullet = bulletPool_.Acquire();
        bullet->Emplace<TelegraphLaser>(
            pos, 
            dir,  // 初始方向（会立即被UpdateDirection覆盖）
            pattern_.laserLength,
            pattern_.laserWidth,
            pattern_.warningDuration,
            pattern_.fireDuration,
            m_playerPos,        // 所有激光传玩家指针
            totalOffset         // 传递偏移角度
        );
        bullet->SetActive(true);
    }
}

void BulletSpawner::FireSingle(const RenderCore::Vector2f& pos, const RenderCore::Vector2f& vel) {
    Bullet* bullet = bulletPool_.Acquire(pos, vel, false);
    bullet->SetActive(true);
}

void BulletSpawner::Update(float deltaTime) {
    // 处理角度增量
    if (pattern_.angleDelta != 0.0f) {
        pattern_.angle += pattern_.angleDelta * deltaTime;
        while (pattern_.angle > 2.0f * M_PI) pattern_.angle -= 2.0f * M_PI;
        while (pattern_.angle < 0.0f) pattern_.angle += 2.0f * M_PI;
    }
    
    // 螺旋模式自动发射
    static float spiralFireTimer = 0.0f;
    if (pattern_.type == BulletPattern::Type::SPIRAL) {
        spiralFireTimer += deltaTime;
        if (spiralFireTimer > 0.1f) { // 每0.05秒发射一发，形成螺旋
            spiralFireTimer = 0.0f;
            
            // 计算螺旋发射位置（围绕一个中心点旋转）
            float radius = pattern_.spiralRadius;
            float spiralAngle = pattern_.angle; // 当前螺旋角度
            
            // 如果radius为0，从同一位置发射；否则围绕中心旋转
            RenderCore::Vector2f emitPos = {
                currentPosition_[0] + std::cos(spiralAngle) * radius,
                currentPosition_[1] + std::sin(spiralAngle) * radius
            };
            
            // 发射子弹
            RenderCore::Vector2f velocity = {
                std::cos(pattern_.angle) * pattern_.speed,
                std::sin(pattern_.angle) * pattern_.speed
            };
            FireSingle(emitPos, velocity);
            
            // 增加螺旋角度
            pattern_.angle += pattern_.spiralSpeed * deltaTime * (M_PI / 180.0f);
        }
    }
}

} // namespace Game