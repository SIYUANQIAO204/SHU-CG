//
// Created by Autumn Sound on 2024/9/23.
//
//裁剪
#ifndef RENDERENGINE_CLIP_HPP
#define RENDERENGINE_CLIP_HPP

#include <variant>

#include "polygon.h"
#include "rectangle.h"

namespace RenderCore {

struct Clip {
    using Window = std::variant<Rectangle, Polygon>;

    bool enable{false};
    Window window;
    enum class Algorithm { CohenSutherland, Midpoint } algorithm{Algorithm::CohenSutherland};
};

}  // namespace RenderCore

#endif  //RENDERENGINE_CLIP_HPP
