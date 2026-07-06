#include "geometry.h"
#include "tgaimage.h"

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_viewport(const int x, const int y, const int w, const int h);
void init_perspective(const double f);
void init_zbuffer(const int w, const int h);

struct IShader {
    static TGAColor sample_uv(const TGAImage &img, const vec2 &uv) {
        return img.get(uv[0]*img.width(),uv[1]*img.height());
    }
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0; //method prototype
};

typedef vec4 Triangle[3];
void rasterize(const Triangle &clip, TGAImage &framebuffer, const IShader &shader);