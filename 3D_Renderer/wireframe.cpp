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
    for (int x = x0; x <= x1; x++) {
        if (!steep) framebuffer.set(x, y, color);
        else framebuffer.set(y, x, color);
        interror += 2 * std::abs(y1 - y0);
        if (interror > x1 - x0) {
            y += y1 > y0 ? 1 : -1;
            interror -= 2 * (x1 - x0);
        }
    }
}

std::tuple<float, float> worldToCamera(vec3 vec) {
    return {(vec.x + 1) * width / 2 , (vec.y + 1) * height / 2};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "path/to/.obj" << std::endl;
        return 1;
    } 

    Model model = Model(argv[1]);
    
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i = 0; i < model.numFaces(); i++) {
        auto [x1, y1] = worldToCamera(model.vert(i, 0));
        auto [x2, y2] = worldToCamera(model.vert(i, 1));
        auto [x3, y3] = worldToCamera(model.vert(i, 2));
        
        drawLine(x1, x2, y1, y2, framebuffer, blue);
        drawLine(x2, x3, y2, y3, framebuffer, blue);
        drawLine(x3, x1, y3, y1, framebuffer, blue);
    }

    for (int i = 0; i < model.numVerts(); i++) {
        auto [x, y] = worldToCamera(model.vert(i));
        framebuffer.set(x, y, green);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}