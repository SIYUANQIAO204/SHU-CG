#pragma once
#include <SDL3/SDL.h>
#include "../include/engine.h"

class Canvas{
    public:
    Canvas(RenderCore::RenderEngine& eng, int w, int h);
    ~Canvas();
    void run();               // 阻塞事件循环
private:
    void present();           // 把 RenderEngine 的位图贴到窗口

    RenderCore::RenderEngine& engine_;
    SDL_Window*   win_  = nullptr;
    SDL_Renderer* ren_  = nullptr;
    SDL_Texture*  tex_  = nullptr;
    int width_, height_;
    bool mouse_down_ = false;
    RenderCore::Point last_;

};