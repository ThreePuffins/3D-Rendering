#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include "geometry.h"
#include "model.h"

constexpr int width  = 1000;
constexpr int height = 1000;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

matrix<4,4, double> Perspective, Viewport, ModelView;

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.},{0, h/2., 0, y+h/2.},{0,0,1,0},{0,0,0,1}}};
}

void perspective(const double f) {
    Perspective = {{{1,0,0,0},{0, 1, 0, 0},{0,0,1,0},{0,0,-1/f,1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    //computes basis vectors for the model from the camera's view
    vec3 n = normalised(eye-center);
    vec3 l = normalised(cross(up,n));
    vec3 m = normalised(cross(n,l));
    //model view converts coordinates from origin O to origin C (the center)
    ModelView = matrix<4,4,double>{{{l.x,l.y,l.z,0},{m.x,m.y,m.z,0},{n.x,n.y,n.z,0},{0,0,0,0}}} *
                matrix<4,4,double>{{{1,0,0,-center.x},{0,1,0,-center.y},{0,0,1,-center.z},{0,0,0,1}}};
}

vec3 rot(vec3 v, double x, double y, double z) {
    matrix<3,3,double>Rx={{{1,0,0},{0,std::cos(x),-std::sin(x)},{0,std::sin(x),std::cos(x)}}};
    matrix<3,3,double>Ry={{{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}}};
    matrix<3,3,double>Rz={{{std::cos(z),-std::sin(z),0},{std::sin(z),std::cos(z),0},{0,0,1}}};
    return Rz*(Ry*(Rx*v));
}

vec3 viewToClipSpace(vec3 v) {
    const double t = 3;
    return v / (1-v.z/t);
}

double signed_triangle_area(int x1, int y1, int x2, int y2, int x3, int y3) {
    return 0.5*((y2-y1)*(x2+x1) + (y3-y2)*(x3+x2) + (y1-y3)*(x1+x3));
}

void drawTriangle(const vec4 clip[3], TGAImage &framebuffer, TGAColor col, std::vector<double> &zbuffer) {
    vec4 ndc[3] = {clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w}; // normalised device coords
    vec2 screen[3] = {(Viewport*ndc[0]).xy(),(Viewport*ndc[1]).xy(),(Viewport*ndc[2]).xy()}; // screen coords
    // TODO: add backface culling
    matrix<3,3,double> ABC = {{{screen[0].x,screen[0].y,1},{screen[1].x,screen[1].y,1},{screen[2].x,screen[2].y,1}}};
    if (ABC.det()<1) return; //backface culling and discards triangles covering less than a pixel
    //find bounding box corners
    auto [box_xmin,box_xmax] = std::minmax({screen[0].x,screen[1].x,screen[2].x});
    auto [box_ymin,box_ymax] = std::minmax({screen[0].y,screen[1].y,screen[2].y});
    #pragma omp parallel for
    for (int x=std::max<int>(box_xmin,0); x<std::min<int>(box_xmax,framebuffer.width()-1); x++) {
        for (int y=std::max<int>(box_ymin,0); y<std::min<int>(box_ymax,framebuffer.height()-1); y++) {
            //https://haqr.eu/tinyrenderer/barycentric/ for barycentric coordinate calculation
            vec3 bc = ABC.invertTransposed() * vec3{(double)x,(double)y,1.};
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            unsigned char z = (bc.x * ndc[0].z + bc.y * ndc[1].z + bc.z * ndc[2].z);
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

    vec3 eye {-1,0,2};
    vec3 center {0,0,0};
    vec3 up {0,1,0};
    lookat(eye, center, up);
    perspective(norm(eye-center));
    viewport(width/16, height/16, width*7/8, height*7/8);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height,-std::numeric_limits<double>::max());

    Model model = Model(argv[1]);
    for (int i = 0; i < model.numFaces(); i++) {
        vec4 clip[3];
        for (int a : {0,1,2}) {
            vec3 v = model.vert(i, a);
            clip[a] = Perspective * ModelView * vec4{v.x,v.y,v.z,1.};
        }

        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        drawTriangle(clip, framebuffer, rnd, zbuffer);
    }

    TGAImage depthbuffer(width, height, TGAImage::GRAYSCALE);

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            depthbuffer.set(x, y, {(unsigned char)zbuffer[x+width*y]});
            
    depthbuffer.write_tga_file("depthbuffer.tga");
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}