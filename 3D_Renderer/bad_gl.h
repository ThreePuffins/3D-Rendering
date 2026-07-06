#include "geometry.h"
#include "tgaimage.h"

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_viewport(const int x, const int y, const int w, const int h);
void init_perspective(const double f);
void init_zbuffer(const int w, const int h);

struct IShader {
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0; //method prototype
};

typedef vec4 Triangle[3];
void rasterize(const Triangle &clip, TGAImage &framebuffer, const IShader &shader);