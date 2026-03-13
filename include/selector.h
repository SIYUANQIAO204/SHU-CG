//
// Created by qiao on 25-11-6.
//
//日后做通用控制器
#pragma once
#ifndef PAINTENGINE_SELECTOR_H
#define PAINTENGINE_SELECTOR_H

#include "bezier.h"
#include "primitive.h"

namespace RenderCore{
    struct ControlPoint {
        RenderCore::Primitive * target = nullptr;     // 当前选中的曲线
        int selectedIndex = -1;       // 当前选中的控制点下标
        float radius = 5.0f;         // 点选判定半径（像素）
    };
}

#endif //PAINTENGINE_SELECTOR_H
