#include "input_map.h"
#include "../include/engine.h"
#include "tools.h"
#include <iostream>
#include <vector>
#include "../include/utils.h"
#include "../include/transform.h"
#include <variant>
#include "../include/geo.h"
static void sync_pen(RenderCore::RenderEngine &eng) {
    RenderCore::PenOptions opt;
    opt.color      = InputMap::inst().color();
    opt.width      = InputMap::inst().width();
    opt.type       = InputMap::inst().pen();
    // 其他字段保持默认即可
    eng.set_pen_options(opt);
}

// 把任意图元从 variant 里拿出来，变换完再原样包回去
static RenderCore::Primitive
make_transformed_prim(const RenderCore::Primitive &src,
                      const RenderCore::Matrix3f &M)
{
    return std::visit(
        [&M](const auto &concrete) -> RenderCore::Primitive {
            using T = std::decay_t<decltype(concrete)>;

            // 跳过非绘制类型
            if constexpr (std::is_same_v<T, std::monostate> ||
                          std::is_same_v<T, RenderCore::PenOptions> ||
                          std::is_same_v<T, RenderCore::Transform> ||
                          std::is_same_v<T, RenderCore::Fill>)   // 按需继续排除
                return concrete;   // 不变

            // 1. 深拷贝
            T transformed = concrete;
            // 2. 原地变换坐标
            RenderCore::apply_transform_matrix(transformed, M);
            // 3. 原样包回 variant
            return transformed;
        },
        src);
}

// ---------- 单例 ----------
InputMap& InputMap::inst() {
    static InputMap obj;
    return obj;
}

InputMap::InputMap() {
    tool_ = makeLineTool(); // 默认工具
}

// ---------- 事件分发 ----------
bool InputMap::handle(SDL_Event& e, RenderCore::RenderEngine& eng) {
    // 为了代码短，用 static 变量记录当前层
    static uint8_t layer = 0; // 0: 基态   'T' 'C' 'W' 'S' 分别代表 Tool/Color/Width/Style 层
    static bool layer_pressed = false;

    if (e.type == SDL_EVENT_KEY_DOWN) {
        uint32_t key = e.key.key;
        // 进入层：只要没在手，一按就进
        if (!layer_pressed) {
            selected_idx_ = -1;
            rotate_mode_geoCenter = false;
            switch (key) {
            case SDLK_T: mode_ = Mode::Normal;layer = 'T'; layer_pressed = true; return true;
            case SDLK_C: mode_ = Mode::Normal;layer = 'C'; layer_pressed = true; return true;
            case SDLK_W: mode_ = Mode::Normal;layer = 'W'; layer_pressed = true; return true;
            case SDLK_S: mode_ = Mode::Normal;layer = 'S'; layer_pressed = true; return true;
            case SDLK_BACKSPACE: eng.clear(); eng.render(); return true;
            case SDLK_D: mode_ = Mode::Normal;layer = 'D'; layer_pressed = true; return true;
            case SDLK_X: mode_ = Mode::Normal;layer = 'X';layer_pressed = true; return true;
            case SDLK_E: mode_ = Mode::Select;layer = 'E';layer_pressed = true; std::cout << "[E] choosemode\n"; return true;
            case SDLK_M: mode_ = Mode::Modify;layer = 'M';layer_pressed = true; std::cout<< "[M] modifymode\n"; return true;
            }
        }
        // 已在某层内：第二键决定具体动作
        if (layer_pressed) {
            switch (layer) {
            // ====== Tool 层 ======
            case 'T':
                switch (key) {
                case SDLK_L: tool_ = makeLineTool(); std::cout << "[T+l] tool -> line\n"; return true;
                case SDLK_C: tool_ = makeCircleTool(); std::cout << "[T+c] tool -> circle\n"; return true;
                case SDLK_A: tool_ = makeArcTool();    std::cout << "[T+a] tool -> arc\n"; break;
                case SDLK_B: tool_ = makeBezierTool(); std::cout << "[T+b] tool -> bezier\n"; break;
                case SDLK_S: tool_ = makeBsplineTool();std::cout << "[T+s] tool -> bspline\n"; break;
                case SDLK_P: tool_ = makePolygenTool();std::cout << "[T+p] tool -> polygen\n"; break;
                }
                break;
            // ====== Color 层 ======
            case 'C':
                switch (key) {
                case SDLK_R: color_ = RenderCore::Colors::Red;   std::cout << "[C+r] color -> red\n"; return true;
                case SDLK_G: color_ = RenderCore::Colors::Green; std::cout << "[C+g] color -> green\n"; return true;
                case SDLK_B: color_ = RenderCore::Colors::Blue;  std::cout << "[C+b] color -> blue\n"; return true;
                case SDLK_W: color_ = RenderCore::Colors::White; std::cout << "[C+w] color -> white\n"; return true;
                }
                break;
            // ====== Width 层 ======
            case 'W':
                switch (key) {
                case SDLK_1: width_ = 1; std::cout << "[W+1] width -> 1\n"; return true;
                case SDLK_2: width_ = 3; std::cout << "[W+2] width -> 3\n"; return true;
                case SDLK_3: width_ = 5; std::cout << "[W+3] width -> 5\n"; return true;
                }
                break;
            // ====== LineStyle 层 ======
            case 'S':
                switch (key) {
                case SDLK_L: pen_options_ = RenderCore::PenOptions::LineType::SOLID; std::cout << "[S+s] style -> SOLID\n"; return true;
                case SDLK_D: pen_options_ = RenderCore::PenOptions::LineType::DASH;   std::cout << "[S+d] style -> DASH\n"; return true;
                case SDLK_T: pen_options_ = RenderCore::PenOptions::LineType::DOT;    std::cout << "[S+t] style -> DOT\n"; return true;
                case SDLK_A: pen_options_ = RenderCore::PenOptions::LineType::DASH_DOT; std::cout << "[S+a] style -> DASH_DOT\n"; return true;
                }
                break;
            //  ====== Fill 层 ======
            case 'D':   
                switch (key) {
                case SDLK_R: mode_ = Mode::Fill; fill_color_ = RenderCore::Colors::Red;   std::cout << "[D+r] fill color -> red\n"; return true;
                case SDLK_G: mode_ = Mode::Fill; fill_color_ = RenderCore::Colors::Green; std::cout << "[D+g] fill color -> green\n"; return true;
                case SDLK_B: mode_ = Mode::Fill; fill_color_ = RenderCore::Colors::Blue;  std::cout << "[D+b] fill color -> blue\n"; return true;
                case SDLK_W: mode_ = Mode::Fill; fill_color_ = RenderCore::Colors::White; std::cout << "[D+w] fill color -> white\n"; return true;
                case SDLK_LEFT:  mode_ = Mode::Fill; fill_tol_ = RenderCore::max(0,fill_tol_-1); std::cout << "[D+←] tol = " << fill_tol_ << '\n'; return true;
                case SDLK_RIGHT: mode_ = Mode::Fill; fill_tol_++; std::cout << "[D+→] tol = " << fill_tol_ << '\n'; return true;
                }
                break;

            case 'X':
                switch (key) {
                case SDLK_R: mode_ = Mode::Clip;clip_sub_ = ClipSubMode::Rect;  clip_pts_.clear(); std::cout << "[X+r] clip -> rect (click 2)\n"; return true;
                case SDLK_P: mode_ = Mode::Clip;clip_sub_ = ClipSubMode::Poly;  clip_pts_.clear(); std::cout << "[X+p] clip -> poly (R-click close)\n"; return true;
                case SDLK_E: // 清除裁剪
                    mode_ = Mode::Clip;
                    eng.set_global_options({.clip={false}});
                    eng.render();
                    std::cout << "[X+e] clip disabled\n";
                    return true;
                }
                break;
            case 'E':
                switch(key){
                case SDLK_R: rotate_mode_geoCenter = true; rotate_mode_random= false; std::cout << "[E+r] choose -> rotate geocentric\n";return true;
                case SDLK_W: rotate_mode_random = true; rotate_mode_geoCenter = false; std::cout<<"[E+w] choose -> rotate random\n";return true;
                default: rotate_mode_geoCenter = false;rotate_mode_random= false; return true;
                }
            } 
        }
    }

    // 只要层键松开，立即回到基态
    if (e.type == SDL_EVENT_KEY_UP) {
        uint32_t key = e.key.key;
        if (layer_pressed &&
            (key == SDLK_T || key == SDLK_C || key == SDLK_W || key == SDLK_S || key == SDLK_D || key == SDLK_X || key == SDLK_E)) {
            layer_pressed = false;
            layer = 0;
            return true;
        }
    }

    // 鼠标事件：喂给当前 tool
    switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (e.button.button == SDL_BUTTON_LEFT) {
            RenderCore::Point p{(int)e.button.x, (int)e.button.y};
            if (mode_ == Mode::Select){
                int hit = eng.pick_primitive(p);
                lDown_ = false;
                selected_idx_ = -1;
                if (hit > 0) {
                    selected_idx_ = hit;
                    selected_prim_copy_  = eng.get_primitives()[hit].get(); // 深拷贝
                    std::cout << "[Pick] idx = " << hit << "\n";
                    eng.render();   // 重画一次，把提示框画出来  
                }
                return true;
            }

            if (mode_ == Mode::Modify && !modify_selected &&! controlpoint_selected){
                int hit = eng.pick_primitive(p);
                //lDown_ = false;
                selected_idx_ = -1;
                if (hit > 0) {
                    selected_idx_ = hit;
                    selected_prim_copy_  = eng.get_primitives()[hit].get(); // 深拷贝
                    const RenderCore::Primitive &pri = eng.get_primitives()[hit];
                    for(auto q:controlpoint_idx_)
                    {
                        eng.remove_primitive(q);
                    }
                    controlpoint_idx_.clear();
                    modify_selected = false;
                    if (auto bezier = std::get_if<RenderCore::BezierCurve>(&pri)) {
                        // bezier 是 const BezierCurve*
                        controlpoint_idx_.push_back(eng.add_primitive(RenderCore::PenOptions{RenderCore::Colors::White,RenderCore::Colors::White,1,RenderCore::PenOptions::LineType::SOLID,}));
                        for (const auto& q : *bezier) {
                            controlpoint_idx_.push_back(eng.add_primitive(RenderCore::make_circle_center_radius(q,10)));
                        }
                        controlpoint_idx_.push_back(eng.add_primitive(RenderCore::PenOptions{color_,RenderCore::Colors::Black,width_,pen_options_,}));
                        modify_selected = true;
                        std::cout << "[Pick] idx = " << hit << "\n";
                        eng.render();   // 重画一次，把提示框画出来
                    }

                }
                return true;
            }

            if (mode_ == Mode::Modify && modify_selected &&!controlpoint_selected)
            {
                int hit = eng.pick_primitive(p,controlpoint_idx_);
                drag_start_ = {(int)e.button.x, (int)e.button.y};
                lDown_ = false;
                selected_controlpoint_idx_ = -1;
                controlpoint_selected = false;
                if (hit > 0) {
                        selected_controlpoint_idx_ = hit;
                        selected_controlpoint_copy_ = eng.get_primitives()[hit].get();
                        std::cout << "[Pick] controlpoint = " << hit << "\n";
                        eng.render();   // 重画一次，把提示框画出来
                        /*for(auto q:controlpoint_idx_)
                        {
                            if(q== selected_controlpoint_idx_) continue;
                            eng.remove_primitive(q);
                        }*/
                        controlpoint_selected = true;
                }
                return true;
            }

            if (mode_ == Mode::Fill) {
                // 直接下填充命令
                RenderCore::PenOptions opt = eng.get_pen_options(); // 复用旧画笔
                opt.fill_color = fill_color_;          // 只改颜色
                eng.set_pen_options(opt);
                eng.add_primitive(make_fill(p));  // 种子填充
                eng.render();
                return true;
            }

            if (mode_ == Mode::Clip) {
                clip_pts_.push_back(p);
                if (clip_sub_ == ClipSubMode::Rect && clip_pts_.size() == 2) {
                    // 矩形裁剪窗口
                    clip_rec = false;
                    RenderCore::Rectangle r{clip_pts_[0], clip_pts_[1]};
                    eng.set_global_options({.clip={true, r}});
                    std::cout << "[X] rect clip set\n";
                    eng.render();
                    clip_pts_.clear();
                }
                if(clip_sub_ == ClipSubMode::Rect && clip_pts_.size() == 1)
                {
                    clip_rec= true;
                }
                if (clip_sub_ == ClipSubMode::Poly && clip_pts_.size() >= 3) {
                    // 多边形裁剪窗口（右键才闭合，这里先不闭合）
                    std::cout << "[X] poly clip now " << clip_pts_.size() << " pts\n";
                    /*auto temp_color =color_;
                    color_ = RenderCore::Colors::White;
                    auto temp_pen_option = pen_options_;
                    pen_options_ = RenderCore::PenOptions::LineType::DOT;
                    int temp2 = eng.add_primitive(RenderCore::PenOptions(color_,color_,3,pen_options_));
                    int temp1 = eng.add_primitive(RenderCore::Polygon(clip_pts_));
                    eng.render();
                    eng.remove_primitive(temp2);
                    eng.remove_primitive(temp1);*/
                }
                return true;
            }

            lDown_ = true;
            last_ = RenderCore::Point{(int)e.button.x, (int)e.button.y};
            tool_->onLButtonDown(eng, last_);
            return true;
        }
        // 右键闭合多边形裁剪窗口
        if (e.button.button == SDL_BUTTON_RIGHT && mode_ == Mode::Clip && clip_sub_ == ClipSubMode::Poly && clip_pts_.size() >= 3) {
            RenderCore::Polygon poly{clip_pts_};
            eng.set_global_options({.clip={true, poly}});
            std::cout << "[X] polygon clip set\n";
            eng.remove_primitive(selected_idx_);
            eng.render();
            clip_pts_.clear();
            return true;
        }

        if (e.button.button == SDL_BUTTON_RIGHT &&!rotate_mode_random && !rotate_mode_geoCenter && mode_ == Mode::Select && selected_idx_ >= 0) {
            dragging_   = true;
            drag_start_ = {(int)e.button.x, (int)e.button.y};
            // 计算包围盒中心
            auto box = eng.get_primitive_bbox(selected_idx_);
            if (box) {
                bbox_center_.x = (box->min_x() + box->max_x()) / 2;
                bbox_center_.y = (box->min_y() + box->max_y()) / 2;
            }
            eng.remove_primitive(selected_idx_);
            eng.render();
            return true;
        }

        if (e.button.button == SDL_BUTTON_RIGHT && !rotate_mode_random && rotate_mode_geoCenter && mode_ == Mode::Select && selected_idx_ >= 0) {
            rotate_ = true;
            drag_start_ = {(int)e.button.x, (int)e.button.y};
                // 计算包围盒中心

                auto box = eng.get_primitive_bbox(selected_idx_);
                if (box) {
                    bbox_center_.x = (box->min_x() + box->max_x()) / 2;
                    bbox_center_.y = (box->min_y() + box->max_y()) / 2;
                    rotate_Center[0]=bbox_center_.x;
                    rotate_Center[1]=bbox_center_.y;
                }
                eng.remove_primitive(selected_idx_);
                eng.render();
                return true;
        }

        if (e.button.button == SDL_BUTTON_RIGHT && rotate_mode_random && !rotate_mode_geoCenter && mode_ == Mode::Select && selected_idx_ >= 0) {
                rotate_ = true;
                drag_start_ = {(int)e.button.x, (int)e.button.y};
                // 计算包围盒中心

                auto box = eng.get_primitive_bbox(selected_idx_);
                if (box) {
                    bbox_center_.x = (box->min_x() + box->max_x()) / 2;
                    bbox_center_.y = (box->min_y() + box->max_y()) / 2;
                }
                rotate_Center={0,0};
                eng.remove_primitive(selected_idx_);
                eng.render();
                return true;
        }

        if (e.button.button == SDL_BUTTON_RIGHT && mode_ == Mode::Modify && modify_selected && controlpoint_selected && selected_idx_ >= 0 && selected_controlpoint_idx_ >= 0)
        {
            modify_selected = false;
            controlpoint_selected = false;
            eng.remove_primitive(controlpoint_idx_[0]-2);
            eng.remove_primitive(controlpoint_idx_[0]+1);
            for(auto q:controlpoint_idx_)
            {
                eng.remove_primitive(q+1);
                eng.remove_primitive(q);
            }
            // eng.remove_primitive(controlpoint_idx_[0]);
            controlpoint_idx_.clear();
            eng.render();
            return true;
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (lDown_ && mode_ == Mode::Normal) {
            last_ = RenderCore::Point{(int)e.motion.x, (int)e.motion.y};
            tool_->onMouseMove(eng, last_);
            return true;
        }

        if (dragging_) {
            RenderCore::Point now{(int) e.motion.x, (int) e.motion.y};
            int dx, dy;
            dx = now.x - drag_start_.x;
            dy = now.y - drag_start_.y;

            int temp1 = eng.add_primitive(RenderCore::make_translate(dx, dy));
            int temp2 = eng.add_primitive(selected_prim_copy_);
            std::cout << "add:" << temp1 << "," << temp2 << std::endl;
            eng.render();
            eng.remove_primitive(temp2);
            eng.remove_primitive(temp1);

            return true;
        }

        if (rotate_) {
                RenderCore::Point now{(int)e.motion.x, (int)e.motion.y};
                //RenderCore::Vector2f geoCenter = RenderCore::Get_geoCenter(selected_prim_copy_);
                float angle1 = std::atan2(drag_start_.y - rotate_Center[1], drag_start_.x - rotate_Center[0]);
                float angle2 = std::atan2(now.y - rotate_Center[1], now.x - rotate_Center[0]);
                float delta = angle2 - angle1;
                std::cout<<delta<<'\n';
                RenderCore::Matrix3f M = RenderCore::make_rotate_martix(delta, rotate_Center);
                RenderCore::Primitive new_prim = make_transformed_prim(selected_prim_copy_, M);
                eng.remove_primitive(selected_idx_);
                eng.insert_primitive(new_prim,selected_idx_);
                //int temp1 = eng.add_primitive(new_prim);
                //int temp1 = eng.add_primitive(RenderCore::make_rotate(delta, {static_cast<int>(rotate_Center[0]),static_cast<int>(rotate_Center[1])}));
                //int temp2 = eng.add_primitive(selected_prim_copy_);
                //std::cout << "add:" <</*temp1 <<*/ ","<< temp2 <<std::endl;
                eng.render();
                //eng.remove_primitive(temp2);
                //eng.remove_primitive(temp1);

                return true;
        }

        if(mode_ == Mode::Clip && clip_rec)
        {
            RenderCore::Point now{(int) e.motion.x, (int) e.motion.y};
            auto temp_color =color_;
            color_ = RenderCore::Colors::White;
            auto temp_pen_option = pen_options_;
            pen_options_ = RenderCore::PenOptions::LineType::DOT;
            int temp2 = eng.add_primitive(RenderCore::PenOptions(color_,fill_color_,3,pen_options_));
            int temp1=eng.add_primitive(RenderCore::make_rectangle(clip_pts_[0],now));
            eng.render();
            eng.remove_primitive(temp1);
            eng.remove_primitive(temp2);
            color_ = temp_color;
            pen_options_ = temp_pen_option;
            return true;
        }

        if(mode_ == Mode::Modify && controlpoint_selected)
        {
            RenderCore::Point now{(int) e.motion.x, (int) e.motion.y};
            int dx, dy;
            dx = now.x - drag_start_.x;
            dy = now.y - drag_start_.y;
            RenderCore::Matrix3f  M = RenderCore::make_translate_matrix(dx,dy);
            RenderCore::Primitive new_prim = make_transformed_prim(selected_controlpoint_copy_, M);
            eng.remove_primitive(selected_controlpoint_idx_);
            eng.render();
            eng.insert_primitive(new_prim,selected_controlpoint_idx_);
            eng.render();
            std::vector<RenderCore::Point> newbezire = {};
            for(int i = 0; i<int(controlpoint_idx_.size());i++)
            {
                const RenderCore::Primitive &pri = eng.get_primitives()[controlpoint_idx_[i]];
                if(auto circle = std::get_if<RenderCore::Circle>(&pri))
                {
                    if(auto c = std::get_if<RenderCore::CircleUseCenterRadius>(circle))
                    {
                        newbezire.push_back(c->center);
                    }
                }
            }
            eng.remove_primitive(selected_idx_);
            eng.insert_primitive(RenderCore::make_bezier_curve(newbezire),selected_idx_);
            eng.render();
            return true;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (e.button.button == SDL_BUTTON_LEFT && mode_ == Mode::Normal) {
            lDown_ = false;
            last_ = RenderCore::Point{(int)e.button.x, (int)e.button.y};
            tool_->onLButtonUp(eng, last_);
            return true;
        }

        if (e.button.button == SDL_BUTTON_RIGHT && dragging_) {
            dragging_ = false;
            RenderCore::Point now{(int)e.button.x, (int)e.button.y};
            int dx = now.x - drag_start_.x;
            int dy = now.y - drag_start_.y;
            // 构造 translate 变换

            // 把偏移乘到副本
            RenderCore::Matrix3f M = RenderCore::make_translate_matrix(dx, dy);
            RenderCore::Primitive new_prim = make_transformed_prim(selected_prim_copy_, M);

            // 3. 重新生成一份“裸坐标”图元并加回引擎
            eng.insert_primitive(new_prim,selected_idx_);

            eng.render();

            // 4. 清空临时状态
            selected_idx_        = -1;
            selected_prim_copy_  = {};
            return true;
        }
            if (e.button.button == SDL_BUTTON_RIGHT && rotate_) {
                rotate_ = false;
                /*RenderCore::Point now{(int)e.button.x, (int)e.button.y};
                // 构造 translate 变换
                RenderCore::Vector2f geoCenter = RenderCore::Get_geoCenter(selected_prim_copy_);
                float angle1 = std::atan2(drag_start_.y - rotate_Center.y, drag_start_.x - rotate_Center.x);
                float angle2 = std::atan2(now.y - rotate_Center.y, now.x - rotate_Center.x);
                float delta = angle2 - angle1;
                // 把偏移乘到副本
                RenderCore::Matrix3f M = RenderCore::make_rotate_martix(delta, rotate_Center);
                RenderCore::Primitive new_prim = make_transformed_prim(selected_prim_copy_, M);

                // 3. 重新生成一份“裸坐标”图元并加回引擎
                eng.insert_primitive(new_prim,selected_idx_);
                */
                eng.render();

                // 4. 清空临时状态
                selected_idx_        = -1;
                selected_prim_copy_  = {};
                return true;
            }
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        if (selected_idx_ >= 0 && !dragging_) {
            float s = e.wheel.y > 0 ? 1.5f : 0.7f; // 滚轮向上放大
            auto box = eng.get_primitive_bbox(selected_idx_);
            if (box) {
                eng.remove_primitive(selected_idx_);
                eng.render();
                int cx = (box->min_x() + box->max_x()) / 2;
                int cy = (box->min_y() + box->max_y()) / 2;
                RenderCore::Vector2f geoCenter={static_cast<float>(cx),static_cast<float>(cy)};
                // 先移到原点，再缩放，再移回去
                //RenderCore::Matrix3f M = RenderCore::make_translate_matrix(-cx, -cy) *RenderCore::make_scale_matrix(s, s) *RenderCore::make_translate_matrix(cx, cy);
                RenderCore::Matrix3f M=RenderCore::make_scale_martix(s,s,geoCenter);
                RenderCore::Primitive new_prim = make_transformed_prim(selected_prim_copy_, M);

                eng.insert_primitive(new_prim,selected_idx_);
                std::cout << "[Scale] s=" << s << "\n";
                eng.render();
            }
            return true;
        }
    }

    return false;
}