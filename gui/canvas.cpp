#include "canvas.h"
#include "../include/line.h"
#include "../include/circle.h"
#include "../include/color.h"
#include "../include/point.h"
#include <iostream>
#include "input_map.h"

Canvas::Canvas(RenderCore::RenderEngine& eng, int w, int h)
    : engine_(eng), width_(w), height_(h) {
    if (!SDL_Init(SDL_INIT_VIDEO)) throw std::runtime_error("SDL_Init failed");
    win_ = SDL_CreateWindow("PaintBoard",
                            w, h, 0);   // SDL3：flag 参数直接砍了
    if (!win_) throw std::runtime_error("SDL_CreateWindow failed");
    ren_ = SDL_CreateRenderer(win_, nullptr); // SDL3：第二个参数是 name，可为 nullptr
    if (!ren_) throw std::runtime_error("SDL_CreateRenderer failed");
    tex_ = SDL_CreateTexture(ren_, SDL_PIXELFORMAT_ABGR8888,
                             SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!tex_) throw std::runtime_error("SDL_CreateTexture failed");
    engine_.init(w, h);
}

Canvas::~Canvas() {
    SDL_DestroyTexture(tex_);
    SDL_DestroyRenderer(ren_);
    SDL_DestroyWindow(win_);
    SDL_Quit();
}

void Canvas::run() {
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = true;
            else InputMap::inst().handle(e,engine_);
        }
        present();
        SDL_Delay(16);
    }
}

void Canvas::present() {
    auto buffer = engine_.get_frame_buffer();
    SDL_UpdateTexture(tex_, nullptr, buffer.data(), width_ * 4);
    SDL_RenderClear(ren_);
    SDL_RenderTexture(ren_, tex_, nullptr, nullptr); // SDL3 新名字
    SDL_RenderPresent(ren_);
}