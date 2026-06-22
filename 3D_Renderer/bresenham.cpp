#include <cmath>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void drawLine(int x0, int x1, int y0, int y1, TGAImage &framebuffer, TGAColor color) {
    // increment rows or columns depending on the gradient
    float gradient;
    if (x1 == x0) gradient = INFINITY; // TODO: make this reasonable
    else gradient = (y1 - y0) / (x1 - x0);
    bool swap = false;
    if (gradient > 1 || gradient < -1) {
        std::swap(x0,y0);
        std::swap(x1,y1);
        swap = true;
    }
    // make sure that start is always on the left
    if (x1 < x0) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }
    for (int x = x0; x < x1; x++) {
        int y = (int)(y0 + (y1 - y0) * (x - x0) / (x1 - x0));
        if (!swap) framebuffer.set(x, y, color);
        else framebuffer.set(y, x, color);
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 128;
    constexpr int height = 128;
    TGAImage framebuffer(width, height, TGAImage::RGB);
    
    
    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    drawLine(ax, ay, bx, by, framebuffer, blue);
    drawLine(cx, cy, bx, by, framebuffer, green);
    drawLine(cx, cy, ax, ay, framebuffer, yellow);
    drawLine(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}