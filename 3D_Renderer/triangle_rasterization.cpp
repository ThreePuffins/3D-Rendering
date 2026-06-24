#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include "vec.h"
#include "model.h"

constexpr int width  = 64;
constexpr int height = 64;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void drawLine(int x0, int x1, int y0, int y1, TGAImage &framebuffer, TGAColor color) {
    // increment rows or columns depending on the gradient
    bool steep = std::abs(x1 - x0) < std::abs(y1 - y0);
    if (steep) {
        std::swap(x0,y0);
        std::swap(x1,y1);
    }
    // make sure that start is always on the left
    if (x1 < x0) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }
    int y = y0;
    float interror = 0;
    for (int x = x0; x < x1; x++) {
        if (!steep) framebuffer.set(x, y, color);
        else framebuffer.set(y, x, color);
        interror += 2 * std::abs(y1 - y0);
        if (interror > x1 - x0) {
            y += y1 > y0 ? 1 : -1;
            interror -= 2 * (x1 - x0);
        }
    }
}

void drawWireTriangle(int x1, int y1, int x2, int y2, int x3, int y3, TGAImage &framebuffer, TGAColor col) {
    drawLine(x1, x2, y1, y2, framebuffer, col);
    drawLine(x2, x3, y2, y3, framebuffer, col);
    drawLine(x3, x1, y3, y1, framebuffer, col);
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, TGAImage &framebuffer, TGAColor col) {
    // sort vertices by y-coords (1 is y big, 3 is y smol)
    if (y1 < y2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    if (y1 < y3) {
        std::swap(x1, x3);
        std::swap(y1, y3);
    }
    if (y2 < y3) {
        std::swap(x2, x3);
        std::swap(y2, y3);
    }

    // rasterize l and r edges
    drawLine(x1, x2, y1, y2, framebuffer, col);
    drawLine(x2, x3, y2, y3, framebuffer, col);
    drawLine(x3, x1, y3, y1, framebuffer, col);
    // since 1 -> 3 is larger y gap, we know that the two sides are between 1-3 and 1-2-3
    
    // draw hor line segments between

    // start from y1, decrement to y3
    float ax = x3, bx = x3;
    int tot_h = y1 - y3;
    float g1 = (float)(x1-x3)/(y1-y3);
    float g2 = (float)(x2-x1)/(y2-y1);
    float g3 = (float)(x2-x3)/(y2-y3);
    for (int y = 0; y < tot_h; y++) {
        ax += g1;
        if (y <= y2 - y3) bx += g3;
        else bx += g2;
        drawLine((int) ax,(int) bx, y + y3, y + y3, framebuffer, col);
    }
    drawLine(x1, x2, y1, y2, framebuffer, white);
    drawLine(x2, x3, y2, y3, framebuffer, white);
    drawLine(x3, x1, y3, y1, framebuffer, white);
}

int main(int argc, char** argv) {

    TGAImage framebuffer(width, height, TGAImage::RGB);


    drawTriangle(40, 12, 3, 4, 25, 50, framebuffer,red);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}