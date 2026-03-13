#include "canvas.h"
#include "../include/engine.h"

int main() {
    RenderCore::RenderEngine engine;
    Canvas canvas(engine, 1960, 1080);
    canvas.run();
    return 0;
}