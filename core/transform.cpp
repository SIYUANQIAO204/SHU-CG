//
// Created by Autumn Sound on 2024/9/30.
//
#include "../include/transform.h"

#include <cmath>
#include <variant>

#include "../include/engine.h"
#include "../include/matrix.h"
using namespace RenderCore;

Matrix3f RenderCore::make_translate_matrix(float x, float y) {
    // 1  0  x
    // 0  1  y
    // 0  0  1
    return {{{1, 0, x}, {0, 1, y}, {0, 0, 1}}};
}

Matrix3f RenderCore::make_rotate_martix(float radian) {
    const auto c = static_cast<float>(std::cos(radian));
    const auto s = static_cast<float>(std::sin(radian));
    // c -s  0
    // s  c  0
    // 0  0  1
    return {{{c, -s, 0}, {s, c, 0}, {0, 0, 1}}};
}

Matrix3f RenderCore::make_rotate_martix(float radian, Vector2f geoCenter)
{
    const auto c = static_cast<float>(std::cos(radian));
    const auto s = static_cast<float>(std::sin(radian));
    const auto x = geoCenter[0];
    const auto y = geoCenter[1];
    return {{{c, -s, x*(1-c)+y*s}, {s, c, y*(1-c)-x*s}, {0, 0, 1}}};
}

Matrix3f RenderCore::make_scale_matrix(float x, float y) {
    // x  0  0
    // 0  y  0
    // 0  0  1
    return {{{x, 0, 0}, {0, y, 0}, {0, 0, 1}}};
}



Matrix3f RenderCore::make_scale_martix(float x, float y, RenderCore::Vector2f geoCenter) {
    return {{x,0,(1-x)*geoCenter[0]},{0,y,(1-y)*geoCenter[1]},{0,0,1}};
}

Matrix3f make_transform_matrix(const Transform &transform,const Vector2f geoCenter) {
    if (std::holds_alternative<Translate>(transform)) {
        const auto &translate = std::get<Translate>(transform);
        return make_translate_matrix(translate.offset.x, translate.offset.y);
    } else if (std::holds_alternative<Rotate>(transform)) {
        const auto &rotate = std::get<Rotate>(transform);
        return /*make_translate_matrix(rotate.center.x, rotate.center.y) *
               make_rotate_matrix(rotate.angle) *
               make_translate_matrix(-rotate.center.x, -rotate.center.y);*/
                make_rotate_martix(rotate.angle, geoCenter);
    } else if (std::holds_alternative<Scale>(transform)) {
        const auto &scale = std::get<Scale>(transform);
        return /*make_translate_matrix(scale.center.x, scale.center.y) *
               make_scale_matrix(scale.scale.x, scale.scale.y) *
               make_translate_matrix(-scale.center.x, -scale.center.y);*/
                make_scale_martix(scale.scale.x,scale.scale.y,geoCenter);
    }
    return Matrix3f::identity();
}

Matrix3f make_transform_matrix(const Transform &transform) {
    if (std::holds_alternative<Translate>(transform)) {
        const auto &translate = std::get<Translate>(transform);
        return make_translate_matrix(translate.offset.x, translate.offset.y);
    } else if (std::holds_alternative<Rotate>(transform)) {
        const auto &rotate = std::get<Rotate>(transform);
        return make_translate_matrix(rotate.center.x, rotate.center.y) *
                make_rotate_martix(rotate.angle) *
               make_translate_matrix(-rotate.center.x, -rotate.center.y);
    } else if (std::holds_alternative<Scale>(transform)) {
        const auto &scale = std::get<Scale>(transform);
        return make_translate_matrix(scale.center.x, scale.center.y) *
               make_scale_matrix(scale.scale.x, scale.scale.y) *
               make_translate_matrix(-scale.center.x, -scale.center.y);
    }
    return Matrix3f::identity();
}

void RenderEngine::make_transform(const Transform &transform,Vector2f geoCenter) {
    transform_matrix_ = make_transform_matrix(transform,geoCenter) * transform_matrix_;
}


void RenderEngine::make_transform(const Transform &transform) {
    transform_matrix_ = make_transform_matrix(transform) * transform_matrix_;
}

bool RenderCore::is_scale_martix(RenderCore::Matrix3f M)
{
    double a = M.m[0][0], b = M.m[0][1];
    double c = M.m[1][0], d = M.m[1][1];

    // translation
    float x = M.m[0][2];
    float y = M.m[1][2];

    // 列范数（注意：列向量是 (a,c) 和 (b,d)）
    double s1 = std::sqrt(a*a + c*c);
    double s2 = std::sqrt(b*b + d*d);

    // 列内积（应该为 0 if orthogonal）
    double dot = a*b + c*d;
    if (!(std::isfinite(s1) && std::isfinite(s2) && std::isfinite(dot))) return false;

    if (std::abs(s1 - s2) > 1e-6) return false;    // 两列长度不一致 -> 非等比缩放
    if (std::abs(dot) > 1e-6) return false;        // 列不正交 -> 存在剪切
    return true;
}

double RenderCore::tsMartix_scale(RenderCore::Matrix3f M) {
    float a = M.m[0][0], b = M.m[0][1];
    float c = M.m[1][0], d = M.m[1][1];

    // translation
    float x = M.m[0][2];
    float y = M.m[1][2];

    // 列范数（注意：列向量是 (a,c) 和 (b,d)）
    float s1 = std::sqrt(a*a + c*c);
    float s2 = std::sqrt(b*b + d*d);
    double s = (s1 + s2) * 0.5;
    return s;
}