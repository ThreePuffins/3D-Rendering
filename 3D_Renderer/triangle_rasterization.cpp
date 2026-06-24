#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include "vec.h"
#include "model.h"

constexpr int width  = 128;
constexpr int height = 128;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

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
    
    int tot_h = y1 - y3;
    if (y2 != y3) {
        int seg_h = y2 - y3;
        for (int y = y3; y <= y2; y++) {
            int ax = x3+((y-y3)*(x1-x3))/tot_h;
            int bx = x3+((y-y3)*(x2-x3))/seg_h;
            for (int x = std::min(ax,bx); x <= std::max(ax,bx); x++) {
                framebuffer.set(x,y,col);
            }
        }
    }
    if (y2 != y1) {
        int seg_h = y1 - y2;
        for (int y = y2; y <= y1; y++) {
            int ax = x3+((y-y3)*(x1-x3))/tot_h;
            int bx = x2+((y-y2)*(x1-x2))/seg_h;
            for (int x = std::min(ax,bx); x <= std::max(ax,bx); x++) {
                framebuffer.set(x,y,col);
            }
        }
    }
}

int main(int argc, char** argv) {

    TGAImage framebuffer(width, height, TGAImage::RGB);


    drawTriangle(50, 20, 12, 10, 30, 50, framebuffer,red);
    drawTriangle(70, 10, 90, 10, 60, 50, framebuffer,green);
    drawTriangle(70, 110, 120, 100, 60, 60, framebuffer,yellow);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}