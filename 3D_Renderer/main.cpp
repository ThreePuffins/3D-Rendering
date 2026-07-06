#include "bad_gl.h"
#include "model.h"

extern matrix<4,4,double> ModelView, Perspective;
extern std::vector<double> zbuffer;   

struct RandomShader : IShader {
    const Model &model;
    TGAColor color = {};
    vec3 tri[3];

    RandomShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);
        vec4 cam_pos = ModelView * vec4{v.x,v.y,v.z,1.};
        tri[vert] = cam_pos.xyz();
        return Perspective * cam_pos;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, color};
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " path/to/.obj" << std::endl;
        return 1;
    } 

    int width = 1000;
    int height = 1000;
    vec3 eye {0,0,2};
    vec3 center {0,0,0};
    vec3 up {0,1,0};

    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width,height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int a = 1; a < argc; a++) {
        Model model = Model(argv[a]);
        RandomShader shader(model);
        for (int i = 0; i < model.numFaces(); i++) {
            shader.color = { (unsigned char)(std::rand()%255), 
                             (unsigned char)(std::rand()%255), 
                             (unsigned char)(std::rand()%255) };
            Triangle clip = { shader.vertex(i, 0),
                              shader.vertex(i, 1),
                              shader.vertex(i, 2) };
            rasterize(clip, framebuffer, shader);
        }
    }

    TGAImage depthbuffer(width, height, TGAImage::GRAYSCALE);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
                depthbuffer.set(x, y, {(unsigned char)(std::min(255*zbuffer[x+width*y],255.))});
    depthbuffer.write_tga_file("depthbuffer.tga");
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}