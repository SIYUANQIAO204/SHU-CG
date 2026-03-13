#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "vector.h"
#include "rectangle.h"
#include "utils.h"

namespace Game {

// 碰撞形状组件（为后续组件化做准备）
struct CircleCollider {
    RenderCore::Vector2f center;
    float radius;
    
    CircleCollider(const RenderCore::Vector2f& c, float r) : center(c), radius(r) {}
};

struct AABBCollider {
    RenderCore::Rectangle bounds;
    
    AABBCollider(const RenderCore::Rectangle& b) : bounds(b) {}
};

// 新增：线段碰撞体（用于激光）
struct LineSegmentCollider {
    RenderCore::Vector2f start;
    RenderCore::Vector2f end;
    float width;  // 线宽（半径）
    
    LineSegmentCollider(const RenderCore::Vector2f& s, const RenderCore::Vector2f& e, float w)
        : start(s), end(e), width(w) {}
};

// 碰撞检测系统
class CollisionSystem {
public:
    // AABB-AABB 碰撞检测（快速排斥）
    static bool CheckAABB(const RenderCore::Rectangle& a, const RenderCore::Rectangle& b) {
        return !(a.max_x() < b.min_x() || a.min_x() > b.max_x() ||
                 a.max_y() < b.min_y() || a.min_y() > b.max_y());
    }
    
    // 圆形-圆形碰撞检测（精确）
    static bool CheckCircles(const CircleCollider& a, const CircleCollider& b) {
        float dx = a.center[0] - b.center[0];
        float dy = a.center[1] - b.center[1];
        float distanceSquared = dx * dx + dy * dy;
        float radiusSum = a.radius + b.radius;
        return distanceSquared <= (radiusSum * radiusSum);
    }
    
    // AABB-圆形 碰撞检测（混合）
    static bool CheckAABBCircle(const AABBCollider& aabb, const CircleCollider& circle) {
        // 找到矩形上离圆心最近的点
        float closestX = RenderCore::max((float)aabb.bounds.min_x(), 
                                         RenderCore::min(circle.center[0], (float)aabb.bounds.max_x()));
        float closestY = RenderCore::max((float)aabb.bounds.min_y(),
                                         RenderCore::min(circle.center[1], (float)aabb.bounds.max_y()));
        
        // 计算距离
        float dx = circle.center[0] - closestX;
        float dy = circle.center[1] - closestY;
        float distanceSquared = dx * dx + dy * dy;
        
        return distanceSquared <= (circle.radius * circle.radius);
    }

     // ✅ 线段-圆形碰撞检测（精确距离计算）
    static bool CheckLineSegmentCircle(const LineSegmentCollider& line, const CircleCollider& circle) {
        // 将线段视为粗线段（胶囊体），检测圆心到线段的距离
        RenderCore::Vector2f lineVec = line.end - line.start;
        RenderCore::Vector2f toCircle = circle.center - line.start;
        
        // 计算线段长度平方
        float lineLenSq = RenderCore::vector_length_square(lineVec);
        
        // 处理零长度线段
        if (lineLenSq < 0.0001f) {
            return RenderCore::vector_length(toCircle) <= (line.width + circle.radius);
        }
        
        // 计算投影参数t
        float t = RenderCore::vector_dot(toCircle, lineVec) / lineLenSq;
        t = RenderCore::max(0.0f, RenderCore::min(1.0f, t)); // 钳制到线段范围内
        
        // 计算线段上最近的点
        RenderCore::Vector2f closestPoint = line.start + lineVec * t;
        
        // 计算圆心到最近点的距离
        float distanceSq = RenderCore::vector_length_square(circle.center - closestPoint);
        float radiusSum = line.width + circle.radius;
        
        return distanceSq <= (radiusSum * radiusSum);
    }
    
    // 通用检测接口（支持未来扩展）
    template<typename T1, typename T2>
    static bool Check(const T1& collider1, const T2& collider2) {
        if constexpr (std::is_same_v<T1, CircleCollider> && std::is_same_v<T2, CircleCollider>) {
            return CheckCircles(collider1, collider2);
        }
        else if constexpr (std::is_same_v<T1, AABBCollider> && std::is_same_v<T2, AABBCollider>) {
            return CheckAABB(collider1.bounds, collider2.bounds);
        }
        else if constexpr (std::is_same_v<T1, AABBCollider> && std::is_same_v<T2, CircleCollider>) {
            return CheckAABBCircle(collider1, collider2);
        }
        else if constexpr (std::is_same_v<T1, CircleCollider> && std::is_same_v<T2, AABBCollider>) {
            return CheckAABBCircle(collider2, collider1);
        }
        else if constexpr (std::is_same_v<T1, LineSegmentCollider> && std::is_same_v<T2, CircleCollider>) {
            return CheckLineSegmentCircle(collider1, collider2);
        }
        else if constexpr (std::is_same_v<T1, CircleCollider> && std::is_same_v<T2, LineSegmentCollider>) {
            return CheckLineSegmentCircle(collider2, collider1);
        }
        return false;
    }
};

} // namespace Game

#endif // COLLISION_SYSTEM_H