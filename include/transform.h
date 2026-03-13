//
// Created by Autumn Sound on 2024/9/30.
//

#ifndef RENDERENGINE_TRANSFORM_HPP
#define RENDERENGINE_TRANSFORM_HPP

#include <type_traits>
#include <variant>

#include "bezier.h"
#include "circle.h"
#include "fill.h"
#include "line.h"
#include "matrix.h"
#include "point.h"
#include "polygon.h"
#include "vector.h"

namespace RenderCore {

Matrix3f make_translate_matrix(float x, float y);
bool is_scale_martix(Matrix3f);
double tsMartix_scale(Matrix3f);
Matrix3f make_scale_matrix(float x, float y);
Matrix3f make_scale_martix(float x,float y,Vector2f geoCenter);
Matrix3f make_rotate_martix(float radian);
Matrix3f make_rotate_martix(float radian, Vector2f geoCenter);
// 平移变换
struct Translate {
    Vector2f offset;
};

inline Translate make_translate(float x, float y) {
    return {{x, y}};
}

// 旋转变换
struct Rotate {
    float angle;
    Point center;
};

inline Rotate make_rotate(float angle, const Point &center) {
    return {angle, center};
}

// 缩放变换
struct Scale {
    Vector2f scale;
    Point center;
};

inline Scale make_scale(float x, float y, const Point &center) {
    return {{x, y}, center};
}

// 变换
using Transform = std::variant<Translate, Rotate, Scale>;

template <typename T>
struct TransformMatrixApply;

template <typename T>
constexpr inline void apply_transform_matrix(T &t, const Matrix3f &transform) {
    TransformMatrixApply<T>{}(t, transform);
}

template <typename T>
using can_apply_transform_matrix = typename TransformMatrixApply<T>::type;

template <typename T>
constexpr bool can_apply_transform_matrix_v = can_apply_transform_matrix<T>::value;

template <typename T>
struct TransformMatrixApply : std::false_type {
    static_assert(!std::is_const_v<T>, "T must not be const");

    constexpr void operator()(T &, const Matrix3f &) const {};
};

template <>
struct TransformMatrixApply<Point> : std::true_type {
    constexpr void operator()(Point &point, const Matrix3f &transform) const {
        if (transform == Matrix3f::identity()) {
            return;
        }
        const auto p = transform * Vector2f(point.x, point.y).xy1();
        point.x = static_cast<int>(p.x);
        point.y = static_cast<int>(p.y);
    }
};

template <>
struct TransformMatrixApply<Line> : std::true_type {
    constexpr void operator()(Line &line, const Matrix3f &transform) const {
        apply_transform_matrix(line.p1, transform);
        apply_transform_matrix(line.p2, transform);
    }
};

template <>
struct TransformMatrixApply<Polygon> : std::true_type {
    //FIXME: 加了constexpr gnu11编译不过
    void operator()(Polygon &polygon, const Matrix3f &transform) const {
        for (auto &point : polygon) {
            apply_transform_matrix(point, transform);
        }
    }
};

template <>
struct TransformMatrixApply<Fill> : std::true_type {
    constexpr void operator()(Fill &fill, const Matrix3f &transform) const {
        apply_transform_matrix(fill.seed, transform);
    }
};

template <>
struct TransformMatrixApply<BezierCurve> : std::true_type {
    //FIXME:  加了constexpr gnu11编译不过
    void operator()(BezierCurve &curve, const Matrix3f &transform) const {
        for (auto &point : curve) {
            apply_transform_matrix(point, transform);
        }
    }
};

// 圆（仅 CenterRadius 模式，ThreePoints 已在 engine 里转为中心+半径）
template <>
struct TransformMatrixApply<Circle> : std::true_type {
    void operator()(Circle &c, const Matrix3f &M) const {
        std::visit([&M](auto &concrete) {
            using T = std::decay_t<decltype(concrete)>;
            if constexpr (std::is_same_v<T, CircleUseCenterRadius>) {
                apply_transform_matrix(concrete.center, M);   // 只移中心
                if(is_scale_martix(M))
                {
                    //std::cout<< tsMartix_scale(M)<<'\n';
                   //change_radius(concrete,int(tsMartix_scale(M)));
                }
            }
            else if constexpr (std::is_same_v<T, CircleUseThreePoints>) {
                apply_transform_matrix(concrete.p1, M);
                apply_transform_matrix(concrete.p2, M);
                apply_transform_matrix(concrete.p3, M);
            }
        }, c);
    }
};

// B 样条：控制点全部变换
template <>
struct TransformMatrixApply<BsplineCurve> : std::true_type {
    void operator()(BsplineCurve &b, const Matrix3f &M) const {
        for (auto &p : b.control_points)
            apply_transform_matrix(p, M);
    }
};

template <>
struct TransformMatrixApply<Arc> : std::true_type {
    void operator()(Arc &a, const Matrix3f &M) const {
        std::visit([&M](auto &concrete) {
            using T = std::decay_t<decltype(concrete)>;
            if constexpr (std::is_same_v<T, ArcUseCenterRadiusAngle>)
                apply_transform_matrix(concrete.center, M);
            else if constexpr (std::is_same_v<T, ArcUseThreePoints>) {
                apply_transform_matrix(concrete.p1, M);
                apply_transform_matrix(concrete.p2, M);
                apply_transform_matrix(concrete.p3, M);
            }
        }, a);
    }
};

}  // namespace RenderCore

#endif  //RENDERENGINE_TRANSFORM_HPP
