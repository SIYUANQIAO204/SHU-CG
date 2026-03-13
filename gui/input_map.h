#pragma once
#include <SDL3/SDL.h>
#include "../include/engine.h"
#include "../include/line.h"
#include "../include/circle.h"
#include "../include/color.h"
#include "../include/primitive.h"
#include "tools.h"

// 前向声明，减少耦合
class InputMap;

enum class Mode { Normal, Fill, Clip, Select,Modify};
enum class ClipSubMode { Rect, Poly };

// 真正管映射的单例
class InputMap {
public:
    static InputMap& inst();

    // 每次帧调用：更新内部状态，返回 true 表示消费了事件
    bool handle(SDL_Event& e, RenderCore::RenderEngine& eng);

    // 实时画笔属性
    const RenderCore::Color& color()      const { return color_; }
    int                      width()      const { return width_; }
    RenderCore::PenOptions::LineType pen()const { return pen_options_; }

private:
    InputMap();  // 私有构造，单例

    // 当前工具
    std::unique_ptr<Tool> tool_;
    ClipSubMode clip_sub_ = ClipSubMode::Rect;
    Mode mode_ = Mode::Normal;

    // 当前属性
    RenderCore::Color color_ = RenderCore::Colors::White;
    RenderCore::Color fill_color_ = RenderCore::Colors::Red;   // 当前填充色
    int               width_ = 1;
    int  fill_tol_   = 0;
    RenderCore::PenOptions::LineType pen_options_ = RenderCore::PenOptions::LineType::SOLID;
    std::vector<RenderCore::Point> clip_pts_;        // 正在采集的裁剪窗口点

    int selected_idx_ = -1;              // 当前选中的图元索引
    bool  rotate_mode_geoCenter = false;     // 是否处于旋转层
    bool  rotate_mode_random = false;
    float rotate_angle_ = 0.0f;     // 累计角度（可选，用于显示）
    bool dragging_   = false;            // 是否正在拖动
    bool rotate_ = false;
    bool clip_rec = false;
    int selected_controlpoint_idx_ =-1;
    RenderCore::Vector2f rotate_Center;
    RenderCore::Point drag_start_;                   // 拖动起始屏幕坐标
    RenderCore::Point drag_last_ = RenderCore::Point{0,0};
    RenderCore::Point bbox_center_;                 // 提示框中心（用于旋转/缩放）
    enum class XFormMode { None, Trans, Rotate } 
    xform_mode_ = XFormMode::None;
    RenderCore::Primitive  selected_prim_copy_;  // 原图元副本（拖动前）
    RenderCore::Primitive  selected_controlpoint_copy_;
    std::vector<int> controlpoint_idx_ = {};
    bool modify_selected = false;
    bool controlpoint_selected = false;
    // 鼠标坐标
    RenderCore::Point last_;
    bool              lDown_ = false;
};

// 具体工具实现放在 cpp 里，这里只暴露类名
struct LineTool;
struct CircleTool;