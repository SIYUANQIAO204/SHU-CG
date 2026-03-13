//
// Created by Autumn Sound on 2024/9/11.
//

#ifndef RENDERENGINE_PRIMITIVE_HPP
#define RENDERENGINE_PRIMITIVE_HPP

#include <variant>

#include "bezier.h"
#include "circle.h"
#include "fill.h"
#include "line.h"
#include "options.h"
#include "polygon.h"
#include "rectangle.h"
#include "transform.h"
namespace RenderCore {

// 图元
// 可以是线段、圆、圆弧、矩形、多边形、填充、画笔选项、变换等
// 也可以是空的 monostate
using Primitive = std::variant<Line, Arc, Rectangle, Polygon, Fill, PenOptions, Transform,
    BezierCurve, BsplineCurve, std::monostate,Circle>;

}

#endif  //RENDERENGINE_PRIMITIVE_HPP
