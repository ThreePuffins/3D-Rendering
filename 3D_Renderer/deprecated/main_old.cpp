#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include "geometry.h"
#include "model.h"

constexpr int width  = 1000;
constexpr int height = 1000;
constexpr vec3 camPos = {0,0,1};

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

vec3 rot(vec3 v, double x, double y, double z) {
    matrix<3,3,double>Rx={{{1,0,0},{0,std::cos(x),-std::sin(x)},{0,std::sin(x),std::cos(x)}}};
    matrix<3,3,double>Ry={{{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}}};
    matrix<3,3,double>Rz={{{std::cos(z),-std::sin(z),0},{std::sin(z),std::cos(z),0},{0,0,1}}};
    return Rz*(Ry*(Rx*v));
}

vec3 viewToClipSpace(vec3 v) {
    return v / (1-v.z/camPos.z);
}

double signed_triangle_area(int x1, int y1, int x2, int y2, int x3, int y3) {
    return 0.5*((y2-y1)*(x2+x1) + (y3-y2)*(x3+x2) + (y1-y3)*(x1+x3));
}

void drawTriangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, 
    TGAImage &framebuffer, TGAColor col, std::vector<double> &zbuffer) {
    //find bounding box corners
    int box_xmax = std::max(x1, std::max(x2, x3));
    int box_xmin = std::min(x1, std::min(x2, x3));
    int box_ymax = std::max(y1, std::max(y2, y3));
    int box_ymin = std::min(y1, std::min(y2, y3));
    double tot_area = signed_triangle_area(x1,y1,x2,y2,x3,y3);
    if (tot_area==0) return;
    #pragma omp parallel for
    for (int x=std::max<int>(box_xmin,0); x<std::min<int>(box_xmax,framebuffer.width()-1); x++) {
        for (int y=std::max<int>(box_ymin,0); y<std::min<int>(box_ymax,framebuffer.height()-1); y++) {
            float alpha = signed_triangle_area(x,y,x2,y2,x3,y3) / tot_area;
            float beta = signed_triangle_area(x,y,x3,y3,x1,y1) / tot_area;
            float gamma = signed_triangle_area(x,y,x1,y1,x2,y2) / tot_area;
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            unsigned char z = (alpha * z1 + beta * z2 + gamma * z3);
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x,y,col);
        }
    }
}

void drawLine(int x0, int y0, int x1, int y1, TGAImage &framebuffer, TGAColor color) {
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

std::tuple<float, float, float> clipToScreenSpace(vec3 vec) {
    return {(vec.x + 1) * width / 2 , 
        (vec.y + 1) * height / 2, 
        (vec.z + 1) * 255 / 2};
}

std::tuple<float, float, float> viewToScreenSpace(vec3 vec) {
    return clipToScreenSpace(viewToClipSpace(vec));
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " path/to/.obj" << std::endl;
        return 1;
    } 

    Model model = Model(argv[1]);
    
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height,-std::numeric_limits<double>::max());

    for (int i = 0; i < model.numFaces(); i++) {
        auto [x1, y1, z1] = viewToScreenSpace(rot(model.vert(i, 0),0,M_PI/3,0) + vec3{0,0,-4});
        auto [x2, y2, z2] = viewToScreenSpace(rot(model.vert(i, 1),0,M_PI/3,0) + vec3{0,0,-4});
        auto [x3, y3, z3] = viewToScreenSpace(rot(model.vert(i, 2),0,M_PI/3,0) + vec3{0,0,-4});

        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;

        drawTriangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, framebuffer, rnd, zbuffer);

    }

    TGAImage depthbuffer(width, height, TGAImage::GRAYSCALE);

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            depthbuffer.set(x, y, {(unsigned char)zbuffer[x+width*y]});
            
    depthbuffer.write_tga_file("depthbuffer.tga");
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}