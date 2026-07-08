#include "bad_gl.h"

matrix<4,4, double> Perspective, Viewport, ModelView;
std::vector<double> zbuffer;

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    //computes basis vectors for the model from the camera's view
    vec3 n = normalised(eye-center);
    vec3 l = normalised(cross(up,n));
    vec3 m = normalised(cross(n, l));
    //model view converts coordinates from origin O to origin C (the center)
    ModelView = matrix<4,4,double>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                matrix<4,4,double>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}
void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.},{0, h/2., 0, y+h/2.},{0,0,1,0},{0,0,0,1}}};
}
void init_perspective(const double f) {
    Perspective = {{{1,0,0,0},{0, 1, 0, 0},{0,0,1,0},{0,0,-1/f,1}}};
}
void init_zbuffer(const int w, const int h) {
    zbuffer = std::vector<double>(w*h,-10000);
}
void rasterize(const Triangle &clip, TGAImage &framebuffer, const IShader &shader) {
    vec4 ndc[3] = {clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w}; // normalised device coords
    vec2 screen[3] = {(Viewport*ndc[0]).xy(),(Viewport*ndc[1]).xy(),(Viewport*ndc[2]).xy()}; // screen coords
    matrix<3,3,double> ABC = {{{screen[0].x,screen[0].y,1},{screen[1].x,screen[1].y,1},{screen[2].x,screen[2].y,1}}};
    if (ABC.det()<1) return; //backface culling and discards triangles covering less than a pixel
    //find bounding box corners
    auto [box_xmin,box_xmax] = std::minmax({screen[0].x,screen[1].x,screen[2].x});
    auto [box_ymin,box_ymax] = std::minmax({screen[0].y,screen[1].y,screen[2].y});
#pragma omp parallel for
    for (int x=std::max<int>(box_xmin,0); x<=std::min<int>(box_xmax,framebuffer.width()-1); x++) {
        for (int y=std::max<int>(box_ymin,0); y<=std::min<int>(box_ymax,framebuffer.height()-1); y++) {
            //https://haqr.eu/tinyrenderer/barycentric/ for barycentric coordinate calculation
            vec3 bc = ABC.invertTransposed() * vec3{(double)x,(double)y,1.};
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            double z = (bc.x * ndc[0].z + bc.y * ndc[1].z + bc.z * ndc[2].z);
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            auto [discard, colour] = shader.fragment(bc);
            if (discard) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x,y,colour);
        }
    }
}