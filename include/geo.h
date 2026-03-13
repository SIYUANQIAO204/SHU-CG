//
// Created by qiao on 25-11-5.
//
#pragma once
#ifndef PAINTENGINE_GEO_H
#define PAINTENGINE_GEO_H

#include "bezier.h"
#include "circle.h"
#include "fill.h"
#include "line.h"
#include "options.h"
#include "polygon.h"
#include "rectangle.h"
#include "transform.h"
#include "vector.h"
#include "primitive.h"

namespace RenderCore{
    inline Vector2f Get_geoCenter(Primitive shape)
    {
        std::visit([](const auto &p)->Vector2f {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
               return {0,0};
            }
            else if constexpr (std::is_same_v<T,Line>)
            {
                int minx = std::min(p.p1.x, p.p2.x);
                int maxx = std::max(p.p1.x, p.p2.x);
                int miny = std::min(p.p1.y, p.p2.y);
                int maxy = std::max(p.p1.y, p.p2.y);
                return {static_cast<float>(float (minx+maxx)/2.0),static_cast<float>(float (miny+maxy)/2.0)};
            }
            else if constexpr (std::is_same_v<T,Polygon>)
            {
                if (p.empty()) return {0, 0};
                float cx= 0, cy = 0;
                for (auto& q : p) {
                    cx += q.x;
                    cy += q.y;
                }
                cx /= p.size();
                cy /= p.size();
                return {cx, cy};
            }
            else if constexpr (std::is_same_v<T,Circle>)
            {
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
                return {static_cast<float>(center.x),static_cast<float>(center.y)};
            }
            else if constexpr ((std::is_same_v<T,Rectangle>))
            {
                return {static_cast<float>(float (p.min_x()+p.max_x())/2.0),static_cast<float>(float (p.min_y()+p.max_y())/2.0)};
            }
            else if constexpr (std::is_same_v<T,Arc>)
            {
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
                return {static_cast<float>(center.x),static_cast<float>(center.y)};
            }
            else if constexpr (std::is_same_v<T, BezierCurve>) {
                if (p.empty()) return {0, 0};
                int minx = p[0].x, maxx = p[0].x;
                int miny = p[0].y, maxy = p[0].y;
                for (const auto &pt: p) {
                    minx = std::min(minx, pt.x);
                    maxx = std::max(maxx, pt.x);
                    miny = std::min(miny, pt.y);
                    maxy = std::max(maxy, pt.y);
                }
                return {static_cast<float>(float(maxx + minx) / 2.0), static_cast<float>(float(maxy + miny) / 2.0)};
            }
            else if constexpr (std::is_same_v<T, BsplineCurve>) {
                if (p.control_points.empty()) return {0,0};
                int minx = p.control_points[0].x, maxx = p.control_points[0].x;
                int miny = p.control_points[0].y, maxy = p.control_points[0].y;
                for (const auto &pt : p.control_points) {
                    minx = std::min(minx, pt.x);
                    maxx = std::max(maxx, pt.x);
                    miny = std::min(miny, pt.y);
                    maxy = std::max(maxy, pt.y);
                }
                return {static_cast<float>(float(maxx + minx) / 2.0), static_cast<float>(float(maxy + miny) / 2.0)};
            }
                // 9. 其他（Fill、PenOptions、Transform）不参与拾取
            else {
                return {0,0};
            }
        },shape);
        return {0,0};
    }
}

#endif //PAINTENGINE_GEO_H
