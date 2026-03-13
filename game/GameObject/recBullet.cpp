#include "recBullet.h"
#include "engine.h"
#include "color.h"

namespace Game {

void recBullet::Update(float deltaTime) {
    if (!active_) return;
    
    // 简单匀速运动
    position_ += velocity_;
    
    // 出界检查（顶端或底端）
    if (position_[1] < -20 || position_[1] > 620) {
        active_ = false; // 标记为非激活，后续移除
    }
}

void recBullet::Render(RenderCore::RenderEngine& engine) {
    if (!active_) return;
    
    // 红色小方块表示子弹
    RenderCore::Rectangle rect = GetBounds();
    engine.set_pen_options({
        .color = isPlayerBullet_ ? RenderCore::Colors::Cyan : RenderCore::Colors::Red,
        .fill_color = isPlayerBullet_ ? RenderCore::Colors::Cyan : RenderCore::Colors::Red
    });
    engine.add_primitive(rect);
}

RenderCore::Rectangle recBullet::GetBounds() const {
    int x = static_cast<int>(position_[0]);
    int y = static_cast<int>(position_[1]);
    int half = size_ / 2;
    return {{x - half, y - half}, {x + half, y + half}};
}

} // namespace Game