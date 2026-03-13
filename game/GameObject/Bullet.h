#pragma once
#include <variant>
#include "recBullet.h"
#include "Laser.h"
#include "TelegraghLaser.h"
#include "../System/CollisionSystem.h"

namespace Game {

    struct Bullet {
        std::variant<recBullet, Laser, TelegraphLaser> data;

        // 默认构造：用 recBullet 的默认构造来初始化 variant
        Bullet() {
            data.emplace<recBullet>(); // 需要 recBullet() 可用；若不可用，改为其他可默认构造的类型
        }

        // 便捷构造：直接构造 recBullet（保持你旧接口）
        Bullet(const RenderCore::Vector2f& pos,
               const RenderCore::Vector2f& vel,
               bool isPlayerBullet)
        {
            data.emplace<recBullet>(pos, vel, isPlayerBullet);
        }

        template<typename T, typename... Args>
        void Emplace(Args&&... args) {
            data.emplace<T>(std::forward<Args>(args)...);
        }

        bool IsActive() const {
            return std::visit([](auto const& b){
                return b.IsActive();
            }, data);
        }

        bool IsLaser() const {
            return std::visit([](auto const& b) -> bool {
                using T = std::decay_t<decltype(b)>;
                //必须同时检查基类和派生类
                if constexpr (std::is_same_v<T, Laser> || std::is_same_v<T, TelegraphLaser>) {
                    return true;
                }
                return false;
            }, data);
        }

        bool Getisplayerbullet() const {
            return std::visit([](auto const& b){
                return b.Getisplayerbullet(); // 保持你子弹类中的命名，或改为统一 GetIsPlayerBullet()
            }, data);
        }

        void SetActive(bool active) {
            std::visit([&](auto& b){
                b.SetActive(active);
            }, data);
        }

        void Update(float dt) {
            std::visit([&](auto& b){
                b.Update(dt);
            }, data);
        }

        void Render(RenderCore::RenderEngine& engine) {
            std::visit([&](auto& b){
                b.Render(engine);
            }, data);
        }

        // 返回内部子弹的位置引用（假设所有子弹都有 const GetPosition() -> Vector2f&）
        const RenderCore::Vector2f& GetPosition() const {
            return std::visit([](auto const& b) -> const RenderCore::Vector2f& {
                // 激光的位置是startPoint，不是普通子弹的动态position_
                using T = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<T, TelegraphLaser>) {
                    return b.GetStartPoint(); // 激光固定起点
                }
                return b.GetPosition();
            }, data);
        }

        // 获取激光的AABB碰撞盒（用于预警激光）
        LineSegmentCollider GetLaserSegment() const {
            return std::visit([](auto const& b) -> LineSegmentCollider {
                using T = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<T, Laser>) {
                    RenderCore::Vector2f end{
                        b.GetStartPoint().x + b.m_direction.x * b.m_length,
                        b.GetStartPoint().y + b.m_direction.y * b.m_length
                    };
                    return {b.GetStartPoint(), end, b.m_width / 2.0f};
                }
                else if constexpr (std::is_same_v<T, TelegraphLaser>) {
                    RenderCore::Vector2f end{
                        b.GetStartPoint().x + b.m_direction.x * b.m_length,
                        b.GetStartPoint().y + b.m_direction.y * b.m_length
                    };
                    float width = (b.m_state == TelegraphLaser::State::Firing) ? b.m_width / 2.0f : 0.5f;
                    return {b.GetStartPoint(), end, width};
                }
                // 其他类型返回无效线段
                return {{0,0}, {0,0}, 0};
            }, data);
        }

        float GetRadius() const {
            return std::visit([](auto const& b) -> float {
                return b.GetRadius();
            }, data);
        }

        RenderCore::Rectangle GetBounds() const {
            return std::visit([](auto const& b){
                return b.GetBounds();
            }, data);
        }

        // 判断弹幕是否处于危险状态（用于预警激光）
        bool IsDangerous() const {
            return std::visit([](auto const& b) -> bool {
                using T = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<T, TelegraphLaser>) {
                    return b.IsDangerous(); // 只有发射阶段才危险
                }
                return true; // 其他弹幕只要激活就危险
            }, data);
        }
    };

} // namespace Game
