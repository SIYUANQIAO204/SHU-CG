//
// Created by Autumn Sound on 2024/9/5.
//
#include <cassert>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <numbers>
#include <ostream>
#include <random>
#include <vector>

#include "include/color.h"
#include "include/engine.h"
#include "include/line.h"
#include "include/matrix.h"
#include "include/point.h"
#include "include/polygon.h"
#include "include/rectangle.h"
#include "include/transform.h"
#include "include/vector.h"

using namespace RenderCore;

static RenderEngine engine;

static constexpr int WIDTH = 800;
static constexpr int HEIGHT = 600;

void lab_1();
void lab_2();
void lab_3();
void lab_4();

void ex_3();

void render_and_save(const std::string &filename) {
    // 计时
    auto start = std::chrono::high_resolution_clock::now();
    engine.render();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s" << std::endl;
    engine.save(filename);
}

void test(std::function<void()> func, const std::string &filename) {
    engine.clear();
    func();
    render_and_save(filename);
}

#define TEST(func)                                                   \
    std::cout << "========= " << #func << " =========" << std::endl; \
    test(func, #func ".bmp");                                        \
    std::cout << std::endl;

int main() {
    engine.init(WIDTH, HEIGHT);

    TEST(lab_1);
    TEST(lab_2);
    TEST(lab_3);
    TEST(lab_4);

    TEST(ex_3);
    return 0;
}

void lab_1() {
    // 红色虚线
    engine.set_pen_options({.color = Colors::Red, .type = PenOptions::LineType::DASH});
    engine.add_primitive(make_line({0, 0}, {799, 599}));
    // 绿色点划线
    engine.set_pen_options({.color = Colors::Green, .type = PenOptions::LineType::DASH_DOT});
    engine.add_primitive(make_line({0, 599}, {799, 0}));
    // 蓝色粗线
    engine.set_pen_options({.color = Colors::Blue, .width = 30});
    engine.add_primitive(make_line({400, 0}, {400, 599}));
    // 黄色点线
    engine.set_pen_options({.color = Colors::Yellow, .type = PenOptions::LineType::DOT});
    engine.add_primitive(make_line({0, 300}, {799, 300}));
    // 白色整圆
    engine.set_pen_options({.color = Colors::White});
    engine.add_primitive(make_circle_center_radius({400, 300}, 200));
    // 青色圆弧
    engine.set_pen_options({.color = Colors::Cyan});
    engine.add_primitive(make_arc_center_radius_angle({400, 300}, 300, 0, std::numbers::pi / 2));
}

void lab_2() {
    engine.set_pen_options({.color = Colors::Red, .fill_color = Colors::Green});
    // 矩形绘制
    engine.add_primitive(make_rectangle({100, 100}, {200, 200}));
    // 多边形绘制
    engine.add_primitive(make_polygon({{300, 100}, {400, 100}, {350, 200}}));
    engine.set_pen_options({.color = Colors::Blue, .fill_color = Colors::Yellow});
    engine.add_primitive(make_polygon({{500, 100}, {600, 100}, {650, 200}, {500, 300}}));
    // 区域填充
    engine.set_pen_options({.color = Colors::Red, .fill_color = Colors::Cyan});
    engine.add_primitive(make_fill({350, 150}));
    // 画点线展示裁剪效果
    // 红色虚线
    engine.set_pen_options({.color = Colors::Red, .type = PenOptions::LineType::DASH});
    engine.add_primitive(make_line({0, 0}, {799, 599}));
    // 绿色点划线
    engine.set_pen_options({.color = Colors::Green, .type = PenOptions::LineType::DASH_DOT});
    engine.add_primitive(make_line({0, 599}, {799, 0}));
    // 蓝色粗线
    engine.set_pen_options({.color = Colors::Blue, .width = 30});
    engine.add_primitive(make_line({400, 0}, {400, 599}));
    // 黄色点线
    engine.set_pen_options({.color = Colors::Yellow, .type = PenOptions::LineType::DOT});
    engine.add_primitive(make_line({0, 300}, {799, 300}));
    // 矩形裁剪
    engine.set_global_options({.clip = {true, make_rectangle({100, 100}, {700, 500})}});
    // 多边形裁剪
    // engine.set_global_options({.clip = {true, make_polygon({})}});
    engine.set_global_options({.clip = {true, make_polygon({{300, 100}, {400, 100}, {350, 200}})}});
}

void lab_3() {
    engine.set_pen_options({.color = Colors::Red});
    // 平移变换
    engine.add_primitive(make_translate(100, 100));
    // 三角形绘制
    engine.add_primitive(make_polygon({{100, 400}, {200, 400}, {150, 500}}));
}

void lab_4() {
    // 贝塞尔曲线绘制
    auto bezier_control_points = std::vector<Point>{{100, 100}, {200, 200}, {300, 100}, {400, 200}};
    engine.set_pen_options({.color = Colors::Red});
    // 绘制控制多边形
    engine.add_primitive(make_polygon(bezier_control_points));
    // 贝塞尔曲线绘制
    engine.set_pen_options({.color = Colors::Yellow});
    engine.add_primitive(make_bezier_curve(bezier_control_points));
    auto bspline_control_points =
        std::vector<Point>{{100, 400}, {200, 500}, {300, 400}, {400, 500}};
    auto bspline_knots = std::vector<float>{0, 1, 2, 3, 4, 5, 6, 7};
    // 绘制控制多边形
    engine.set_pen_options({.color = Colors::Red});
    engine.add_primitive(make_polygon(bspline_control_points));
    // b样条曲线绘制
    engine.set_pen_options({.color = Colors::Yellow});
    engine.add_primitive(make_bspline_curve(bspline_control_points, bspline_knots));
}

void ex_3() {
    engine.set_pen_options({.color = Colors::Red});
    // 先经过平移（平移量为（1，0）
    // 后经过旋转（旋转角度为+30度）
    // 平移变换
    engine.add_primitive(make_translate(100, 0));
    // 旋转变换
    engine.add_primitive(make_rotate(30.0 / 180.0 * std::numbers::pi, {0, 0}));
    // 三角形绘制
    engine.add_primitive(make_polygon({{100, 100}, {200, 100}, {150, 200}}));

    engine.add_primitive(make_line({0, 0}, {100, 100}));

    extern Matrix3f make_transform_matrix(const Transform &transform);

    auto t1 = make_transform_matrix(make_translate(1, 0));
    auto t2 = make_transform_matrix(make_rotate(30.0 / 180.0 * std::numbers::pi, {0, 0}));
    auto t3 = t2 * t1;
    std::cout << t3 << std::endl;
    //P（2，1）经过平移（1，0）和旋转（30度）后的坐标
    auto p1 = t3 * Vector2f(2, 1).xy1();
    std::cout << p1.xy() << std::endl;

    std::cout << "----------------" << std::endl;

    // 先经过旋转（旋转角度为+30度）后经过平移（平移量为（1，0）
    auto t4 = t1 * t2;
    std::cout << t4 << std::endl;
    //P（2，1）经过旋转（30度）和平移（1，0）后的坐标
    auto p2 = t4 * Vector2f(2, 1).xy1();
    std::cout << p2.xy() << std::endl;
}