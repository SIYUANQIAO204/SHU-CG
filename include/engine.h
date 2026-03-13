//
// Created by Autumn Sound on 2024/9/5.
//

#ifndef RENDERENGINE_ENGINE_HPP
#define RENDERENGINE_ENGINE_HPP

#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

#include "bitmap.h"
#include "line.h"
#include "matrix.h"
#include "options.h"
#include "point.h"
#include "polygon.h"
#include "primitive.h"
#include "transform.h"
#include "vector.h"
#include "geo.h"
#include <array>

namespace RenderCore {
class RenderEngine;
}

class RenderCore::RenderEngine {
    // 帧缓冲区
    // 调用 render() 时，将绘制到此缓冲区
    std::unique_ptr<Bitmap> frame_buffer_;

    // 存储所有图元
    // Primitive 是一个变体类型，可以存储多种图元
    // PenOptions 类型的图元会使接下来的图元使用指定的画笔选项，直到下一个 PenOptions 类型的图元
    // Transform 类型的图元会使接下来的可绘制图元使用指定的变换矩阵，仅对接下来的一个图元有效
    // 多个 Transform 类型的图元会叠加变换矩阵
    // 如：[..., T1, T2, L1, ...] 代表先对 L1 进行 T1 变换，再对结果进行 T2 变换
    std::vector<Primitive> primitives_;

    // 存储需要渲染的图元
    // 渲染时，会先将 primitives_ 中的图元复制到 render_primitives_ 中
    // 然后遍历 render_primitives_ 进行渲染
    // 因为可能对图元进行裁剪等操作，所以需要复制一份
    std::list<Primitive> render_primitives_;

    // 画布宽高
    int32_t width_;
    int32_t height_;

    // 画笔选项
    // 在渲染过程自动更新
    // 设置成成员变量方便在渲染过程中获取画笔选项
    PenOptions pen_options_;

    // 全局选项
    // 指定背景色、裁剪窗口等
    GlobalOptions global_options_;

    // 变换矩阵
    // 用于对图元进行变换
    Matrix3f transform_matrix_ = Matrix3f::identity();

    // 需要重新渲染
    // 在添加、修改图元时设置为 true
    bool need_render_{true};

   public:
    using Buffer = Bitmap::Buffer;

   public:
    RenderEngine() : frame_buffer_(nullptr), width_(0), height_(0) {}

    RenderEngine(int32_t width, int32_t height)
        : frame_buffer_(nullptr), width_(width), height_(height) {
        init(width, height);
    }

    // 初始化
    void init() { init(width_, height_); }

    // 以指定宽高初始化
    void init(int32_t width, int32_t height) {
        width_ = width;
        height_ = height;
        frame_buffer_ = std::make_unique<Bitmap>(width, height);
        fill_with_background_color();
    }

    // 清空画布，以背景色填充
    void fill_with_background_color() {
        const auto color = vector_to_color(global_options_.background_color);
        if (frame_buffer_) {
            frame_buffer_->fill(color);
        }
    }

    // 初始化画布
    void clear() {
        primitives_.clear();
        global_options_ = {};
        fill_with_background_color();
    }

    void set_global_options(const GlobalOptions &options) {
        global_options_ = options;
        need_render_ = true;
    }

    int pick_primitive(const Point &p) const {
        // 简单暴力：按绘制顺序倒序测，第一个碰到就中
        for (int i = (int)primitives_.size() - 1; i >= 0; --i) {
            auto bbox = get_transformed_bbox(i);
            if (!bbox) continue;
            if (p.x >= bbox->min_x() && p.x <= bbox->max_x() &&
            p.y >= bbox->min_y() && p.y <= bbox->max_y())
                return i;
        }
        return -1;
    }

    int pick_primitive(const Point &p,std::vector<int> range){
        for (int i = (int)primitives_.size() - 1; i >= 0; --i) {
            auto bbox = get_transformed_bbox(i);
            if (!bbox) continue;
            if (p.x >= bbox->min_x() && p.x <= bbox->max_x() &&
                p.y >= bbox->min_y() && p.y <= bbox->max_y())
                for(auto q:range)
                {
                    if(i==q) return i;
                }
        }
        return -1;
    }

    std::optional<Rectangle> get_primitive_bbox(int idx) const
    {
        if (idx < 0 || idx >= (int)primitives_.size())
            return std::nullopt;

        const Primitive &prim = primitives_[idx];

        // 用 visitor 模式逐个图元处理
        return std::visit([](const auto &p) -> std::optional<Rectangle> {
            using T = std::decay_t<decltype(p)>;

            // 1. 空图元
            if constexpr (std::is_same_v<T, std::monostate>) {
                return std::nullopt;
            }
            // 2. 线段
            else if constexpr (std::is_same_v<T, Line>) {
                int minx = std::min(p.p1.x, p.p2.x);
                int maxx = std::max(p.p1.x, p.p2.x);
                int miny = std::min(p.p1.y, p.p2.y);
                int maxy = std::max(p.p1.y, p.p2.y);
                return Rectangle{{minx, miny}, {maxx, maxy}};
            }
            // 3. 矩形
            else if constexpr (std::is_same_v<T, Rectangle>) {
                return p;   // 本身就是矩形
            }
            // 4. 多边形
            else if constexpr (std::is_same_v<T, Polygon>) {
                if (p.empty()) return std::nullopt;
                int minx = p[0].x, maxx = p[0].x;
                int miny = p[0].y, maxy = p[0].y;
                for (const auto &pt : p) {
                    minx = std::min(minx, pt.x);
                    maxx = std::max(maxx, pt.x);
                    miny = std::min(miny, pt.y);
                    maxy = std::max(maxy, pt.y);
                }
                return Rectangle{{minx, miny}, {maxx, maxy}};
            }
            // 5. 圆（两种表示都转成中心+半径，再求 bbox）
            else if constexpr (std::is_same_v<T, Circle>) {
                Point center;
                int   radius = 0;
                if (std::holds_alternative<CircleUseCenterRadius>(p)) {
                    const auto &c = std::get<CircleUseCenterRadius>(p);
                    center = c.center;
                    radius = c.radius;
                } else {
                    const auto &c = std::get<CircleUseThreePoints>(p);
                    auto [ctr, r] = circle_center_radius(c.p1, c.p2, c.p3);
                    center = ctr;
                    radius = r;
                }
                return Rectangle{
                    {center.x - radius, center.y - radius},
                    {center.x + radius, center.y + radius}
                };
            }
            // 6. 圆弧（同样先搞出中心+半径）
            else if constexpr (std::is_same_v<T, Arc>) {
                Point center;
                int   radius = 0;
                if (std::holds_alternative<ArcUseCenterRadiusAngle>(p)) {
                    const auto &a = std::get<ArcUseCenterRadiusAngle>(p);
                    center = a.center;
                    radius = a.radius;
                } else {
                    const auto &a = std::get<ArcUseThreePoints>(p);
                    auto [ctr, r] = circle_center_radius(a.p1, a.p2, a.p3);
                    center = ctr;
                    radius = r;
                }
                return Rectangle{
                    {center.x - radius, center.y - radius},
                    {center.x + radius, center.y + radius}
                };
            }
            // 7. BezierCurve
            else if constexpr (std::is_same_v<T, BezierCurve>) {
                if (p.empty()) return std::nullopt;
                int minx = p[0].x, maxx = p[0].x;
                int miny = p[0].y, maxy = p[0].y;
                for (const auto &pt : p) {
                    minx = std::min(minx, pt.x);
                    maxx = std::max(maxx, pt.x);
                    miny = std::min(miny, pt.y);
                    maxy = std::max(maxy, pt.y);
                }
                return Rectangle{{minx, miny}, {maxx, maxy}};
            }
            // 8. BsplineCurve
            else if constexpr (std::is_same_v<T, BsplineCurve>) {
                if (p.control_points.empty()) return std::nullopt;
                int minx = p.control_points[0].x, maxx = p.control_points[0].x;
                int miny = p.control_points[0].y, maxy = p.control_points[0].y;
                for (const auto &pt : p.control_points) {
                    minx = std::min(minx, pt.x);
                    maxx = std::max(maxx, pt.x);
                    miny = std::min(miny, pt.y);
                    maxy = std::max(maxy, pt.y);
                }
                return Rectangle{{minx, miny}, {maxx, maxy}};
            }
            // 9. 其他（Fill、PenOptions、Transform）不参与拾取
            else {
                return std::nullopt;
            }
        }, prim);
    }

    std::optional<Rectangle> get_transformed_bbox(int idx) const
    {
        if (idx < 0 || idx >= (int)primitives_.size())
            return std::nullopt;

        const Primitive &prim = primitives_[idx];

        // 1. 取出原图元包围盒（未变换）
        auto raw_box = get_primitive_bbox(idx);
        if (!raw_box) return std::nullopt;

        // 2. 构造变换矩阵（跟你在 input_map 里用的完全一致）
        //    注意：这里我们只考虑“最后一次”的 transform_matrix_
        //    因为 input_map 里每次拖动都“乘完即清”，所以这里用当前矩阵即可
        const Matrix3f &M = transform_matrix_;

        // 3. 把矩形的四个角点乘一遍矩阵，再求新的极值
        std::array<Vector2f,4> corners = {
            Vector2f{(float)raw_box->min_x(), (float)raw_box->min_y()},
            Vector2f{(float)raw_box->max_x(), (float)raw_box->min_y()},
            Vector2f{(float)raw_box->max_x(), (float)raw_box->max_y()},
            Vector2f{(float)raw_box->min_x(), (float)raw_box->max_y()}
        };

        int new_minx = INT_MAX, new_maxx = INT_MIN;
        int new_miny = INT_MAX, new_maxy = INT_MIN;

        for (auto &c : corners) {
            auto t = M * c.xy1();          // 3×3 矩阵乘齐次坐标
            int  x = static_cast<int>(std::round(t.x));
            int  y = static_cast<int>(std::round(t.y));
            new_minx = std::min(new_minx, x);
            new_maxx = std::max(new_maxx, x);
            new_miny = std::min(new_miny, y);
            new_maxy = std::max(new_maxy, y);
        }

        return Rectangle{{new_minx, new_miny}, {new_maxx, new_maxy}};
    }

    [[nodiscard]] const GlobalOptions &get_global_options() const { return global_options_; }

    [[nodiscard]] const PenOptions &get_pen_options() const { return pen_options_; }

    // 绘制像素
    void draw_pixel(int x, int y, const Color &color) {
        if (!frame_buffer_) {
            return;
        }
        // 忽略超出范围的点
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            return;
        }

        // 变换
        // 变换矩阵不是单位矩阵时，对点进行变换
        if (transform_matrix_ != Matrix3f::identity()) {
            const auto point = transform_matrix_ * Vector2f(x, y).xy1();
            x = static_cast<int>(point.x);
            y = static_cast<int>(point.y);

            // 重新检查，防止变换后超出范围
            if (x < 0 || x >= width_ || y < 0 || y >= height_) {
                return;
            }
        }

        // 颜色混合
        const auto bg_color = color_to_vector(frame_buffer_->get_pixel(x, y));
        const auto alpha = color.a;
        const auto new_color = color * alpha + bg_color * (1 - alpha);

        // 将新颜色写入帧缓冲区
        frame_buffer_->set_pixel(x, y, vector_to_color(new_color));
    }

    void draw_pixel(int x, int y, uint32_t color) { draw_pixel(x, y, color_to_vector(color)); }

    // 保存到文件
    void save(const std::string &filename) {
        if (frame_buffer_) {
            frame_buffer_->save_bmp(filename.c_str());
        }
    }

    // 获取帧缓冲区
    [[nodiscard]] Buffer get_frame_buffer() const {
        if (frame_buffer_) {
            return frame_buffer_->save_to_buffer();
        }
        return {};
    }

    // 绘制图元
    int add_primitive(const Primitive &primitive) {
        if (!frame_buffer_) {
            return -1;
        }
        need_render_ = true;
        primitives_.push_back(primitive);
        return primitives_.size() - 1;
    }

    // 插入图元
    void insert_primitive(const Primitive &primitive, size_t index) {
        if (!frame_buffer_) {
            return;
        }
        need_render_ = true;
        if (index < primitives_.size()) {
            primitives_.insert(primitives_.begin() + index, primitive);
        } else {
            primitives_.push_back(primitive);
        }
    }

    // 移除图元
    void remove_primitive(size_t index) {
        if (!frame_buffer_) {
            return;
        }
        need_render_ = true;
        if (index < primitives_.size()) {
            primitives_.erase(primitives_.begin() + index);
        }
    }

    // 移除图元(默认)
    void remove_primitive(){
        if (!frame_buffer_) {
            return;
        }
        need_render_ = true;
        primitives_.pop_back();
    }

    // 修改图元
    void modify_primitive(size_t index, const Primitive &primitive) {
        if (!frame_buffer_) {
            return;
        }
        need_render_ = true;
        if (index < primitives_.size()) {
            primitives_[index] = primitive;
        }
    }

    // 获取图元
    [[nodiscard]] std::vector<std::reference_wrapper<const Primitive>> get_primitives() const {
        return {primitives_.begin(), primitives_.end()};
    }

    // 设置画笔选项
    void set_pen_options(const PenOptions &options) { add_primitive(options); }

    // 应用变换矩阵
    // 对于能在栅格化前应用变换矩阵的图元，直接应用变换矩阵
    template <typename T>
    void apply_transform(T &t) {
        if constexpr (can_apply_transform_matrix_v<T>) {
            apply_transform_matrix(t, transform_matrix_);
            transform_matrix_ = Matrix3f::identity();
        }
    }

    // 渲染
    bool render() {
        if (!frame_buffer_) {
            return false;
        }
        if (!need_render_) {
            return true;
        }
        fill_with_background_color();
        // 重置画笔选项
        pen_options_ = {};

        // 复制图元
        render_primitives_ = std::list<Primitive>(primitives_.begin(), primitives_.end());
        for (auto primitive : render_primitives_) {
            // 应用变换矩阵
            // 对于能在栅格化前应用变换矩阵的图元，直接对原始图元应用变换矩阵
            // 对于不能在栅格化前应用变换矩阵的图元，先保存变换矩阵，在栅格化时通过draw_pixel应用变换矩阵
            std::visit(
                [this](auto &prim) {
                    Vector2f geoCenter= Get_geoCenter(prim);
                    using T = std::decay_t<decltype(prim)>;
                    if constexpr (std::is_same_v<T, Transform>) {
                        make_transform(prim,geoCenter);
                    } else {
                        apply_transform(prim);
                    }
                },
                primitive);
            // 裁剪
            // 对于需要裁剪的图元，进行裁剪
            // 裁剪后的图元会替换原始图元
            // 裁剪后的图元可能是空的 monostate
            clip(primitive);
            // 开始进行栅格化
            // 对于不同的图元，使用不同的栅格化算法
            // 栅格化后的图元会根据画笔选项进行绘制
            // 如果是画笔选项，只更新画笔选项
            // 画笔选项会影响接下来的图元直到下一个画笔选项
            std::visit(
                [this](const auto &prim) {
                    using T = std::decay_t<decltype(prim)>;
                    if constexpr (std::is_same_v<T, Line>) {
                        draw_line(prim);
                    } else if constexpr (std::is_same_v<T, Circle>) {
                        draw_circle(prim);
                    } else if constexpr (std::is_same_v<T, Arc>) {
                        draw_arc(prim);
                    } else if constexpr (std::is_same_v<T, Rectangle>) {
                        draw_rectangle(prim);
                    } else if constexpr (std::is_same_v<T, Polygon>) {
                        draw_polygon(prim);
                    } else if constexpr (std::is_same_v<T, Fill>) {
                        draw_fill(prim);
                    } else if constexpr (std::is_same_v<T, PenOptions>) {
                        pen_options_ = prim;
                    } else if constexpr (std::is_same_v<T, BezierCurve>) {
                        draw_bezier_curve(prim);
                    } else if constexpr (std::is_same_v<T, BsplineCurve>) {
                        draw_bspline_curve(prim);
                    };
                    // 重置变换矩阵
                    // 变换矩阵只对下一个图元有效
                    if constexpr (!std::is_same_v<T, Transform> && !std::is_same_v<T, PenOptions>) {
                        transform_matrix_ = Matrix3f::identity();
                    }
                },
                primitive);
        }
        need_render_ = false;
        return true;
    }

   private:
    // 绘制点
    // 线型线宽的控制也在这里实现
    // index 用于控制虚线、点线、点划线等
    void draw_point(int x, int y, int index = -1);

    // 绘制线段
    void draw_line(const Line &line);

    // 绘制圆
    void draw_circle(const Circle &circle);

    // 绘制圆弧
    void draw_arc(const Arc &arc);

    // 绘制矩形
    void draw_rectangle(const Rectangle &rectangle);

    // 绘制多边形
    void draw_polygon(const Polygon &polygon);

    // 填充
    void draw_fill(const Fill &fill);

    // 裁剪
    void clip(Primitive &primitive);

    // 变换
    void make_transform(const Transform &transform,Vector2f geoCenter);
    void make_transform(const Transform &transform);

    // 贝塞尔曲线
    void draw_bezier_curve(const BezierCurve &curve);

    // B样条曲线
    void draw_bspline_curve(const BsplineCurve &curve);

   private:
    // DDA 算法绘制线段
    void draw_line_dda(const Point &start, const Point &end);

    // 中点算法绘制线段
    void draw_line_midpoint(const Point &start, const Point &end);

    // Bresenham 算法绘制线段
    void draw_line_bresenham(const Point &start, const Point &end);

    // 中点画圆算法
    void draw_circle_midpoint(const Point &center, int radius);

    // 中点画圆弧算法
    void draw_arc_midpoint(const Point &center, int radius, float start_angle, float end_angle);

    // 扫描线算法绘制多边形
    void draw_polygon_scanline(const Polygon &polygon);

    // 种子填充算法填充多边形
    void fill_polygon_seedfill(const Fill &fill);

    // 矩形窗口裁剪
    void rectangle_clip(const Rectangle &window, Primitive &primitive);

    // 任意凸多边形裁剪
    void polygon_clip(const Polygon &window, Primitive &primitive);

    // Cohen-Sutherland 裁剪算法
    void clip_line_cohen_sutherland(const Rectangle &window, std::optional<Line> &line);

    // 中点分割裁剪算法
    void clip_line_midpoint(const Rectangle &window, std::optional<Line> &line);

    // 裁剪多边形
    // Sutherland-Hodgman
    void clip_sutherland_hodgman(const Polygon &window, std::optional<Polygon> &polygon);
};

#endif  //RENDERENGINE_ENGINE_HPP
