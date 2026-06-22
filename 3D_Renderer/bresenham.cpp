#include <cmath>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv) {
    constexpr int width  = 128;
    constexpr int height = 128;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int x0 = 7, y0 = 20, x1 = 10, y1 = 100;
    
    drawLine(x0, x1, y0, y1, framebuffer, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

void drawLine(int x0, int x1, int y0, int y1, TGAImage &framebuffer, TGAColor color) {
    bool swap = false;
    if ((y1 - y0) / (x1 - x0) > 1) {
        std::swap(x0,y0);
        std::swap(x1,y1);
        swap = true;
    }
    for (int x = x0; x < x1; x++) {
        int y = (int)(y0 + (y1 - y0) * (x - x0) / (x1 - x0));
        if (!swap) framebuffer.set(x, y, white);
        else framebuffer.set(y, x, white);
    }
}