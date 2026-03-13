// TrajectoryController.h
#ifndef TRAJECTORY_CONTROLLER_H
#define TRAJECTORY_CONTROLLER_H

#include "../include/vector.h"
#include "../include/bezier.h"
#include <random>
#include <vector>

namespace Game {

    enum class TrajectoryType {
        LINEAR,    // 直线
        ARC,       // 圆弧
        BEZIER     // 贝塞尔曲线
    };

    struct Trajectory {
        TrajectoryType type;
        RenderCore::Vector2f start;    // 起点
        RenderCore::Vector2f end;      // 终点

        // 圆弧特有参数
        RenderCore::Vector2f arcCenter; // 圆心
        float startAngle;               // 起始角度(弧度)
        float endAngle;                 // 结束角度(弧度)
        float radius;                   // 半径

        // 贝塞尔曲线特有参数
        std::vector<RenderCore::Vector2f> controlPoints; // 控制点

        // 新增：默认构造函数（保持与旧代码兼容）
        Trajectory() = default;
        
        // 新增：直线轨迹构造函数
        Trajectory(TrajectoryType t, RenderCore::Vector2f s, RenderCore::Vector2f e)
            : type(t), start(s), end(e) {}
        
        // 新增：圆弧轨迹构造函数
        Trajectory(TrajectoryType t, RenderCore::Vector2f s, RenderCore::Vector2f e, 
                RenderCore::Vector2f center)
            : type(t), start(s), end(e), arcCenter(center) {}
        
        // 新增：贝塞尔轨迹构造函数（关键修复）
        // 使用std::initializer_list支持嵌套初始化列表
        Trajectory(TrajectoryType t, RenderCore::Vector2f s, RenderCore::Vector2f e,
                std::initializer_list<RenderCore::Vector2f> cps)
            : type(t), start(s), end(e), controlPoints(cps) {}
    };

    class TrajectoryController {
    private:
        Trajectory currentTraj;
        float progress = 0.0f;          // 轨迹进度(0-1)
        float speed = 0.1f;             // 移动速度(进度/秒)

    public:
        TrajectoryController() = default;

        // 随机生成轨迹类型
        static TrajectoryType RandomType() {
            int type = rand() % 3;
            return static_cast<TrajectoryType>(type);
        }

        // 创建直线轨迹
        void SetLinearTrajectory(const RenderCore::Vector2f& start,
                                const RenderCore::Vector2f& end);

        // 创建圆弧轨迹
        void SetArcTrajectory(const RenderCore::Vector2f& start,
                             const RenderCore::Vector2f& end,
                             const RenderCore::Vector2f& center);

        // 创建贝塞尔曲线轨迹
        void SetBezierTrajectory(const RenderCore::Vector2f& start,
                                const RenderCore::Vector2f& end,
                                const std::vector<RenderCore::Vector2f>& controlPoints);

        // 更新轨迹位置
        RenderCore::Vector2f Update(float deltaTime);

        // 检查轨迹是否完成
        bool IsCompleted() const { return progress >= 1.0f; }

        // 重置轨迹
        void Reset() { progress = 0.0f; }

        // 设置移动速度
        void SetSpeed(float newSpeed) { speed = newSpeed; }
    };

} // namespace Game

#endif // TRAJECTORY_CONTROLLER_H