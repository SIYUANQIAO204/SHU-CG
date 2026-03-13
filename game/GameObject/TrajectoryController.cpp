// TrajectoryController.cpp
#define _USE_MATH_DEFINES
#include "TrajectoryController.h"
#include <cmath>
#include <functional>

namespace Game {

void TrajectoryController::SetLinearTrajectory(const RenderCore::Vector2f& start,
                                              const RenderCore::Vector2f& end) {
    currentTraj.type = TrajectoryType::LINEAR;
    currentTraj.start = start;
    currentTraj.end = end;
    progress = 0.0f;
}

void TrajectoryController::SetArcTrajectory(const RenderCore::Vector2f& start,
                                           const RenderCore::Vector2f& end,
                                           const RenderCore::Vector2f& center) {
    currentTraj.type = TrajectoryType::ARC;
    currentTraj.start = start;
    currentTraj.end = end;
    currentTraj.arcCenter = center;

    // 计算半径
    currentTraj.radius = RenderCore::vector_length(start - center);

    // 计算起始角度和结束角度
    currentTraj.startAngle = std::atan2(start[1] - center[1], start[0] - center[0]);
    currentTraj.endAngle = std::atan2(end[1] - center[1], end[0] - center[0]);

    progress = 0.0f;
}

void TrajectoryController::SetBezierTrajectory(const RenderCore::Vector2f& start,
                                              const RenderCore::Vector2f& end,
                                              const std::vector<RenderCore::Vector2f>& controlPoints) {
    currentTraj.type = TrajectoryType::BEZIER;
    currentTraj.start = start;
    currentTraj.end = end;
    currentTraj.controlPoints = controlPoints;

    // 确保控制点包含起点和终点
    if (!currentTraj.controlPoints.empty() && currentTraj.controlPoints.front() != start) {
        currentTraj.controlPoints.insert(currentTraj.controlPoints.begin(), start);
    }
    if (!currentTraj.controlPoints.empty() && currentTraj.controlPoints.back() != end) {
        currentTraj.controlPoints.push_back(end);
    }

    progress = 0.0f;
}

RenderCore::Vector2f TrajectoryController::Update(float deltaTime) {
    // 更新进度
    progress += speed * deltaTime;
    if (progress > 1.0f) progress = 1.0f;

    switch (currentTraj.type) {
        case TrajectoryType::LINEAR: {
            // 直线插值
            return currentTraj.start + (currentTraj.end - currentTraj.start) * progress;
        }

        case TrajectoryType::ARC: {
            // 圆弧插值
            float angle = currentTraj.startAngle +
                         (currentTraj.endAngle - currentTraj.startAngle) * progress;
            return {
                currentTraj.arcCenter[0] + std::cos(angle) * currentTraj.radius,
                currentTraj.arcCenter[1] + std::sin(angle) * currentTraj.radius
            };
        }

        case TrajectoryType::BEZIER: {
            // 贝塞尔曲线插值（使用现有引擎的De Casteljau算法）
            if (currentTraj.controlPoints.size() < 2) {
                return currentTraj.start;
            }

            std::function<RenderCore::Vector2f(const std::vector<RenderCore::Vector2f>&, float)> de_casteljau =
                [&](const std::vector<RenderCore::Vector2f>& points, float t) -> RenderCore::Vector2f {
                if (points.size() == 1) {
                    return points[0];
                }
                std::vector<RenderCore::Vector2f> new_points;
                for (size_t i = 0; i < points.size() - 1; i++) {
                    new_points.push_back((1 - t) * points[i] + t * points[i + 1]);
                }
                return de_casteljau(new_points, t);
            };

            return de_casteljau(currentTraj.controlPoints, progress);
        }
    }

    return currentTraj.start;
}

} // namespace Game