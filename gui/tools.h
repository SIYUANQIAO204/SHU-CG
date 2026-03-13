#pragma once
#include "../include/engine.h"
#include "../include/point.h"

// 基类
struct Tool {
    virtual ~Tool() = default;
    virtual void onLButtonDown(RenderCore::RenderEngine& eng,
                               const RenderCore::Point& p) = 0;
    virtual void onMouseMove  (RenderCore::RenderEngine& eng,
                               const RenderCore::Point& p) = 0;
    virtual void onLButtonUp  (RenderCore::RenderEngine& eng,
                               const RenderCore::Point& p) = 0;
    virtual void onRButtonDown (RenderCore::RenderEngine& eng,
                               const RenderCore::Point& p) = 0;
};

// 工厂函数（input_map.cpp 里只调这些，无需知道类名）
std::unique_ptr<Tool> makeLineTool();
std::unique_ptr<Tool> makeCircleTool();
std::unique_ptr<Tool> makeArcTool();      // 三点弧
std::unique_ptr<Tool> makeBezierTool();   // 四次贝塞尔
std::unique_ptr<Tool> makeBsplineTool();  // 四次 B 样条
std::unique_ptr<Tool> makePolygenTool();  // 任意五边形