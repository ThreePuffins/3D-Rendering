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

double signed_triangle_area(int x1, int y1, int x2, int y2, int x3, int y3) {
    return 0.5*((y2-y1)*(x2+x1) + (y3-y2)*(x3+x2) + (y1-y3)*(x1+x3));
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, TGAImage &framebuffer, TGAColor col) {
    //find bounding box corners
    int box_xmax = std::max(x1, std::max(x2, x3));
    int box_xmin = std::min(x1, std::min(x2, x3));
    int box_ymax = std::max(y1, std::max(y2, y3));
    int box_ymin = std::min(y1, std::min(y2, y3));
    double tot_area = signed_triangle_area(x1,y1,x2,y2,x3,y3);
    #pragma omp parallel for
    for (int x = box_xmin; x < box_xmax; x++) {
        for (int y = box_ymin; y < box_ymax; y++) {
            float alpha = signed_triangle_area(x,y,x1,y1,x2,y2) / tot_area;
            float beta = signed_triangle_area(x,y,x2,y2,x3,y3) / tot_area;
            float gamma = signed_triangle_area(x,y,x3,y3,x1,y1) / tot_area;
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            framebuffer.set(x,y,col);
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