#pragma once
#include "../include/point.h"
#include "../include/color.h"
#include "../include/rectangle.h"
#include "../include/polygon.h"
#include "../include/engine.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Game {

class GameFont {
public:
    static GameFont& Instance();
    
    void RenderText(RenderCore::RenderEngine& engine, 
                   const std::string& text, 
                   const RenderCore::Point& position, 
                   const RenderCore::Color& color,
                   float scale = 1.0f);
    
    int GetTextWidth(const std::string& text, float scale = 1.0f) const;
    
private:
    GameFont() = default;
    
    void RenderChar(RenderCore::RenderEngine& engine, char c, 
                   const RenderCore::Point& position, 
                   const RenderCore::Color& color, float scale);
    
    struct CharDefinition {
        std::vector<RenderCore::Point> points;
        int width;
    };
    
    std::unordered_map<char, CharDefinition> charDefinitions_;
    void InitializeChars();
};

} // namespace Game