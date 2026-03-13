#include "GameFont.h"

namespace Game {

GameFont& GameFont::Instance() {
    static GameFont instance;
    return instance;
}

void GameFont::InitializeChars() {
    if (!charDefinitions_.empty()) return;
    // A
    charDefinitions_['A'] = {
        {
            {3,1},
            {2,2}, {4,2},
            {1,3}, {5,3},
            {1,4}, {2,4}, {3,4}, {4,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {5,6}
        }, 6
    };

    // B (优化，粗大竖体+两个弧)
    charDefinitions_['B'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, 
            {1,2}, {5,2},
            {1,3}, {2,3}, {3,3}, {4,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {2,6}, {3,6}, {4,6}
        }, 6
    };
    // C
    charDefinitions_['C'] = {
        {
            {2,1}, {3,1}, {4,1}, 
            {1,2}, {5,2},
            {1,3},
            {1,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    // D
    charDefinitions_['D'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {2,6}, {3,6}, {4,6}
        }, 6
    };
    // E
    charDefinitions_['E'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {1,2},
            {1,3}, {2,3}, {3,3},
            {1,4},
            {1,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    // F
    charDefinitions_['F'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {1,2},
            {1,3}, {2,3}, {3,3},
            {1,4},
            {1,5},
            {1,6}
        }, 6
    };
    // G
    charDefinitions_['G'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3},
            {1,4}, {3,4}, {4,4}, {5,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    // H
    charDefinitions_['H'] = {
        {
            {1,1}, {5,1},
            {1,2}, {5,2},
            {1,3}, {2,3}, {3,3}, {4,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {5,6}
        }, 6
    };
    // I
    charDefinitions_['I'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {3,2},
            {3,3},
            {3,4},
            {3,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    // J
    charDefinitions_['J'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {3,2},
            {3,3},
            {3,4},
            {1,5}, {3,5},
            {2,6}
        }, 6
    };
    // K
    charDefinitions_['K'] = {
        {
            {1,1}, {5,1},
            {1,2}, {4,2},
            {1,3}, {3,3},
            {1,4}, {2,4},
            {1,5}, {4,5},
            {1,6}, {5,6}
        }, 6
    };
    // L
    charDefinitions_['L'] = {
        {
            {1,1},
            {1,2},
            {1,3},
            {1,4},
            {1,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    // M
    charDefinitions_['M'] = {
        {
            {1,1}, {5,1},
            {1,2}, {2,2}, {4,2}, {5,2},
            {1,3}, {3,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {5,6}
        }, 6
    };
    // N
    charDefinitions_['N'] = {
        {
            {1,1}, {5,1},
            {1,2}, {2,2}, {5,2},
            {1,3}, {3,3}, {5,3},
            {1,4}, {4,4}, {5,4},
            {1,5}, {5,5},
            {1,6}, {5,6}
        }, 6
    };
    // O
    charDefinitions_['O'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    // P
    charDefinitions_['P'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {2,3}, {3,3}, {4,3},
            {1,4},
            {1,5},
            {1,6}
        }, 6
    };
    // Q
    charDefinitions_['Q'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {4,5},
            {2,6}, {3,6}, {5,6},
            {4,7}
        }, 7
    };
    // R
    charDefinitions_['R'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1},
            {1,2}, {4,2},
            {1,3}, {3,3},
            {1,4}, {2,4}, {3,4},
            {1,5}, {4,5},
            {1,6}, {5,6}
        }, 6
    };
    // S
    charDefinitions_['S'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2},
            {1,3}, {2,3}, {3,3},
            {4,4},
            {3,5}, {4,5},
            {4,6}, {3,6}, {2,6}, {1,6}
        }, 6
    };
    // T
    charDefinitions_['T'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {3,2},
            {3,3},
            {3,4},
            {3,5},
            {3,6}
        }, 6
    };
    // U
    charDefinitions_['U'] = {
        {
            {1,1}, {5,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {1,4}, {5,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    // V
   charDefinitions_['V'] = {
        {
            {1,1},{5,1},
            {1,2},{5,2},
            {2,3},{4,3},
            {2,4},{4,4},
            {3,6}
        }, 6
    };
    // W
    charDefinitions_['W'] = {
        {
            {1,1}, {5,1},
            {1,2}, {3,2}, {5,2},
            {1,3}, {3,3}, {5,3},
            {2,4}, {4,4},
            {2,5}, {4,5},
            {3,6}
        }, 6
    };
    // X
    charDefinitions_['X'] = {
        {
            {1,1}, {5,1},
            {2,2}, {4,2},
            {3,3},
            {2,4}, {4,4},
            {1,5}, {5,5}
        }, 6
    };
    // Y
    charDefinitions_['Y'] = {
        {
            {1,1}, {5,1},
            {2,2}, {4,2}, {3,3},
            {3,4},
            {3,5},
            {3,6}
        }, 6
    };
    // Z
    charDefinitions_['Z'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {5,2},
            {4,3},
            {3,4},
            {2,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    
    // 数字 0-9
    charDefinitions_['0'] = charDefinitions_['O'];
    
    charDefinitions_['1'] = {
        {
            {2,1}, {3,1},
            {1,2}, {3,2},
            {3,3},
            {3,4},
            {3,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    
    charDefinitions_['2'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {5,3},
            {4,4},
            {3,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6}
        }, 6
    };
    
    charDefinitions_['3'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {5,3},
            {3,3}, {4,3},
            {5,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    
    charDefinitions_['4'] = {
        {
            {4,1},
            {3,2}, {4,2},
            {2,3}, {4,3},
            {1,4}, {4,4},
            {1,5}, {2,5}, {3,5}, {4,5}, {5,5},
            {4,6}
        }, 6
    };
    
    charDefinitions_['5'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {1,2},
            {1,3}, {2,3}, {3,3}, {4,3},
            {5,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    
    charDefinitions_['6'] = {
        {
            {3,1}, {4,1},
            {2,2}, {5,2},
            {1,3},
            {1,4}, {2,4}, {3,4}, {4,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    
    charDefinitions_['7'] = {
        {
            {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {5,2},
            {4,3},
            {3,4},
            {2,5},
            {1,6}
        }, 6
    };
    
    charDefinitions_['8'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {2,4}, {3,4}, {4,4},
            {1,5}, {5,5},
            {2,6}, {3,6}, {4,6}
        }, 6
    };
    
    charDefinitions_['9'] = {
        {
            {2,1}, {3,1}, {4,1},
            {1,2}, {5,2},
            {1,3}, {5,3},
            {2,4}, {3,4}, {4,4}, {5,4},
            {5,5},
            {1,6}, {4,6}
        }, 6
    };
    
    // 标点符号
    charDefinitions_[':'] = {
        {
            {2,2}, {3,2},
            {2,5}, {3,5}
        }, 4
    };
    
    // 空格
    charDefinitions_[' '] = {{}, 3};

    // 感叹号
    charDefinitions_['!'] = {
        { 
        {3,1}, {3,2}, 
        {3,3}, {3,5} }, 4 
    };
}

void GameFont::RenderText(RenderCore::RenderEngine& engine, 
                         const std::string& text, 
                         const RenderCore::Point& position, 
                         const RenderCore::Color& color,
                         float scale) {
    InitializeChars();
    
    int x = position.x;
    int y = position.y;
    
    for (char c : text) {
        RenderChar(engine, c, {x, y}, color, scale);
        auto it = charDefinitions_.find(c);
        if (it != charDefinitions_.end()) {
            x += (it->second.width + 1) * scale; // 字符宽度 + 间距
        } else {
            x += 4 * scale; // 默认宽度
        }
    }
}

void GameFont::RenderChar(RenderCore::RenderEngine& engine, char c, 
                         const RenderCore::Point& position, 
                         const RenderCore::Color& color, float scale) {
    auto it = charDefinitions_.find(c);
    if (it == charDefinitions_.end()) {
        return;
    }
    
    const auto& charDef = it->second;
    engine.set_pen_options({.color = color, .fill_color = color});
    
    // 为每个点绘制小方块
    for (const auto& point : charDef.points) {
        int px = position.x + point.x * scale;
        int py = position.y + point.y * scale;
        RenderCore::Rectangle pixel{
        {px, py}, 
        {px + static_cast<int>(scale), py + static_cast<int>(scale)}
};
        engine.add_primitive(pixel);
    }
}

int GameFont::GetTextWidth(const std::string& text, float scale) const {
    int width = 0;
    for (char c : text) {
        auto it = charDefinitions_.find(c);
        if (it != charDefinitions_.end()) {
            width += (it->second.width + 1) * scale;
        } else {
            width += 4 * scale;
        }
    }
    return width;
}

} // namespace Game