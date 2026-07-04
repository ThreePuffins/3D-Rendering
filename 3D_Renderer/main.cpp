#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include "geometry.h"
#include "model.h"

constexpr int width  = 200;
constexpr int height = 200;

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
    ModelView = matrix<4,4,double>{{{l.x,l.y,l.z,0},{m.x,m.y,m.z,0},{n.x,n.y,n.z,0},{0,0,0,1}}} *
                matrix<4,4,double>{{{1,0,0,-center.x},{0,1,0,-center.y},{0,0,1,-center.z},{0,0,0,1}}};
}

vec3 rot(vec3 v, double x, double y, double z) {
    matrix<3,3,double>Rx={{{1,0,0},{0,std::cos(x),-std::sin(x)},{0,std::sin(x),std::cos(x)}}};
    matrix<3,3,double>Ry={{{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}}};
    matrix<3,3,double>Rz={{{std::cos(z),-std::sin(z),0},{std::sin(z),std::cos(z),0},{0,0,1}}};
    return Rz*(Ry*(Rx*v));
}

void drawTriangle(const vec4 clip[3], TGAImage &framebuffer, TGAColor col, std::vector<double> &zbuffer) {
    vec4 ndc[3] = {clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w}; // normalised device coords
    vec2 screen[3] = {(Viewport*ndc[0]).xy(),(Viewport*ndc[1]).xy(),(Viewport*ndc[2]).xy()}; // screen coords
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

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " path/to/.obj" << std::endl;
        return 1;
    } 

    vec3 eye {-2,0,0};
    vec3 center {0,0,0};
    vec3 up {0,1,0};
    lookat(eye, center, up);
    perspective(norm(eye-center));
    viewport(width/16, height/16, width*7/8, height*7/8);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height,-std::numeric_limits<double>::max());

    // Model model = Model(argv[1]);
    // for (int i = 0; i < model.numFaces(); i++) {
    //     vec4 clip[3];
    //     for (int a : {0,1,2}) {
    //         vec3 v = model.vert(i, a);
    //         clip[a] = Perspective * ModelView * vec4{v.x,v.y,v.z,1.};
    //     }

    //     TGAColor rnd;
    //     for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
    //     drawTriangle(clip, framebuffer, rnd, zbuffer);
    // }


    vec4 clip[3];
    clip[0] = Perspective * (ModelView * vec4{0,1,0,1.});
    clip[1] = Perspective * (ModelView * vec4{-0.5,0.5,0,1.});
    clip[2] = Perspective * (ModelView * vec4{0.5,-0.5,0,1.});


    for(int i=0;i<3;i++)
        for(int j=0;j<4;j++)
            std::cerr << "vert: " << i << " axis: " << j << " value: "<< clip[i][j] << std::endl;
    
    TGAColor rnd;
    for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
    drawTriangle(clip, framebuffer, rnd, zbuffer);


    TGAImage depthbuffer(width, height, TGAImage::GRAYSCALE);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            depthbuffer.set(x, y, {(unsigned char)zbuffer[x+width*y]});

    
            
    depthbuffer.write_tga_file("depthbuffer.tga");
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}