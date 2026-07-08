#include "bad_gl.h"
#include "model.h"

extern matrix<4,4,double> ModelView, Perspective, Viewport;
extern std::vector<double> zbuffer;   

struct RandomShader : IShader {
    const Model &model;
    vec3 tri[3];

    RandomShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        vec4 v = model.vert(face, vert);
        vec4 cam_pos = ModelView * vec4{v.x,v.y,v.z,1.};
        tri[vert] = cam_pos.xyz();
        return Perspective * cam_pos;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor color = { (unsigned char)(std::rand()%255), 
                  (unsigned char)(std::rand()%255), 
                  (unsigned char)(std::rand()%255) };
        return {false, color};
    }
};

struct PhongShader : IShader {
    const Model &model;
    vec4 tri[3];
    vec4 vn[3];
    vec4 l;

    PhongShader(const Model &m, const vec3 light) : model(m) {
        //TODO: make it a point light cuz that'd be cool 
        //directional light i think
        l = normalised((ModelView * vec4{light.x,light.y,light.z,1.})); 
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec4 v = model.vert(face, vert);
        vec4 cam_pos = ModelView * vec4{v.x,v.y,v.z,1.};
        tri[vert] = cam_pos;
        vec4 n = model.vertNormal(face, vert);
        // cant just apply modelview transformation to normals, have to transform it with invert transposed modelview instead
        vn[vert] = ModelView.invertTransposed() * vec4{n.x, n.y, n.z, 0.};
        return Perspective * cam_pos;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        vec4 n = normalised(bar.x * vn[0] + bar.y * vn[1] + bar.z * vn[2]);
        vec4 r = 2.*n*(n*l)-l; // wrote a lil proof for this irl :)

        TGAColor frag_col = {150,100,244};
        double ambient = .3;
        double diffuse = std::max(n * l,0.); // dot product is more efficient than cos
        // bc modelview makes the z axis parallel to the camera-eye vector, the dot product of the reflection and said vector is just the z component of r
        double specular = std::pow(std::max(r.z,0.),30);
        for (int c : {0,1,2}) {
            frag_col[c] *= std::min(1.,ambient + 0.4*diffuse + specular);
        }
        return {false, frag_col};
    }
};

// phong shader which implements shadow mapping
struct PhongShader_nm : IShader {
    const Model &model;
    vec4 l;
    vec2 vert_uv[3];
    vec4 tri[3];
    vec4 vn[3];
    int shadow_w;
    int shadow_h;
    std::vector<double> shadow_mask;
    matrix<4,4,double> light_transform;

    vec4 screen_space[3];

    PhongShader_nm(const Model &m, const vec3 light, const std::vector<double> &s_mask, 
        const int s_w, const int s_h, const matrix<4,4,double> &l_transform) : model(m), 
        shadow_mask(s_mask), shadow_w(s_w), shadow_h(s_h), light_transform(l_transform) {
        //TODO: make it a point light cuz that'd be cool
        l = normalised((ModelView * vec4{light.x,light.y,light.z,1.})); 
    }

    virtual vec4 vertex(const int face, const int vert) {
        vert_uv[vert] = model.uv(face,vert);
        tri[vert] = ModelView * model.vert(face,vert);
        vn[vert] = ModelView.invertTransposed() * model.vertNormal(face,vert);

        screen_space[vert] = Viewport * Perspective * ModelView * model.vert(face,vert);

        return Perspective * tri[vert];
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        // TODO: (maybe) try and figure out why the below commented code didn't work
        // vec4 frag = ModelView.invert() * (bar[0] * tri[0] + bar[1] * tri[1] + bar[2] * tri[2]); // world coords
        vec4 frag = (Viewport* Perspective * ModelView).invert() * (bar.x * screen_space[0] + bar.y * screen_space[1] + bar.z * screen_space[2]); // world coords
        vec4 q = light_transform * frag;
        vec3 p = q.xyz()/q.w;
        bool lit = (p.x<0 || p.x>=shadow_w || p.y<0 || p.y>=shadow_h) || // outside of shadow buffer
            (p.z > shadow_mask[int(p.x) + int(p.y)*shadow_w] - .03); // add small bias for z-fighting

        vec2 uv = bar.x * vert_uv[0] + bar.y * vert_uv[1] + bar.z * vert_uv[2];
        matrix<2,4,double> E = {tri[0]-tri[1],tri[1]-tri[2]};
        matrix<2,2,double> U = {vert_uv[0]-vert_uv[1],vert_uv[1]-vert_uv[2]};
        matrix<2,4,double> T = {U.invert() * E}; //both tangent and bitangent vectors, mapping globalspace to uv vectors
        matrix<4,4,double> tangent_space_basis = {T[0],T[1],
            normalised(bar.x * vn[0] + bar.y * vn[1] + bar.z * vn[2]), {0,0,0,1}};
        // created the tangent_space_basis transposed to begin with, so must detranspose it now
        vec4 n = normalised(tangent_space_basis.transpose() * model.normal(uv));
        vec4 r = 2.*n*(n*l)-l; // wrote a lil proof for this irl :)

        TGAColor frag_col = sample_uv(model.diffuse(),uv);
        double ambient = .3;
        TGAColor glow_uv = sample_uv(model.glow(),uv);
        if (lit) {
            double diffuse = .9*std::max(n * l,0.); // dot product is more efficient than cos
            // bc modelview makes the z axis parallel to the camera-eye vector, the dot product of the reflection and said vector is just the z component of r
            double specular= std::pow(std::max(r.z,0.),30);
            TGAColor specular_uv = sample_uv(model.specular(),uv);
            for (int c : {0,1,2}) {
                frag_col[c] = std::min<int>(255,frag_col[c]*(3.*glow_uv[c]/255.
                    + ambient + lit* (diffuse + specular * (255 + 3. * specular_uv[c]/255.))));
            }
        }
        else {
            for (int c : {0,1,2}) {
                frag_col[c] = std::min<int>(255,frag_col[c]*(3.*glow_uv[c]/255. + ambient));
            }
        }
        return {false, frag_col};
    }
};

struct BlankShader : IShader {
    const Model &model;
    BlankShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        return Perspective * ModelView * model.vert(face, vert);
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, {255,255,255}};
    }
};

// derives from the basic phong shader but also implements shadow mapping
struct ToonShader : IShader {
    const Model &model;
    vec4 l;
    vec4 tri[3];
    vec4 vn[3];
    int shadow_w;
    int shadow_h;
    std::vector<double> shadow_mask;
    matrix<4,4,double> light_transform;

    vec4 screen_space[3];

    ToonShader(const Model &m, const vec3 light, const std::vector<double> &s_mask, 
        const int s_w, const int s_h, const matrix<4,4,double> &l_transform) : model(m), 
        shadow_mask(s_mask), shadow_w(s_w), shadow_h(s_h), light_transform(l_transform) {
        //TODO: make it a point light cuz that'd be cool
        l = normalised((ModelView * vec4{light.x,light.y,light.z,1.})); 
    }

    virtual vec4 vertex(const int face, const int vert) {
        tri[vert] = ModelView * model.vert(face, vert);
        // cant just apply modelview transformation to normals, have to transform it with invert transposed modelview instead
        vn[vert] = ModelView.invertTransposed() * model.vertNormal(face,vert);
        screen_space[vert] = Viewport * Perspective * ModelView * model.vert(face,vert);
        return Perspective * tri[vert];
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {

        vec4 frag = (Viewport* Perspective * ModelView).invert() * (bar.x * screen_space[0] + bar.y * screen_space[1] + bar.z * screen_space[2]); // world coords
        vec4 q = light_transform * frag;
        vec3 p = q.xyz()/q.w;
        bool lit = (p.x<0 || p.x>=shadow_w || p.y<0 || p.y>=shadow_h) || // outside of shadow buffer
            (p.z > shadow_mask[int(p.x) + int(p.y)*shadow_w] - .03); // add small bias for z-fighting

        vec4 n = normalised(bar.x * vn[0] + bar.y * vn[1] + bar.z * vn[2]);
        vec4 r = 2.*n*(n*l)-l; // wrote a lil proof for this irl :)

        vec3 frag_col = {250,100,254}; // bgr
        double ambient = .2;
        double diffuse = 1.4 * std::max(n * l,0.); // dot product is more efficient than cos
        // bc modelview makes the z axis parallel to the camera-eye vector, the dot product of the reflection and said vector is just the z component of r
        double specular = 4 * std::pow(std::max(r.z,0.),30);
        double intensity = std::min(1.,ambient + diffuse * lit + specular * lit);
        double bands = 4;
        for (double i = 0; i <= bands; i++) {
            if (intensity < i / bands) {
                intensity = i / bands;
                break;
            }
        }

        frag_col = frag_col * intensity;

        return {false, TGAColor{(unsigned char)frag_col[0],
            (unsigned char)frag_col[1],(unsigned char)frag_col[2]}};
    }
};

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cerr << "Usage: " << argv[0] << " path/to/.obj" << std::endl;
        return 1;
    }

    int width = 1000;
    int height = 1000;
    int s_width = 1000;
    int s_height = 1000;
    vec3 eye {-1,0,2};
    vec3 center {0,0,0};
    vec3 up {0,1,0};
    vec3 sun {1,1,1};

    bool edge_detection = false;

    //shadow rendering pass
    lookat(sun, center, up);
    init_perspective(norm(sun-center));
    init_viewport(s_width/16, s_height/16, s_width*7/8, s_height*7/8);
    init_zbuffer(s_width,s_height);
    TGAImage trash(s_width, s_height, TGAImage::GRAYSCALE);

    for (int a = 1; a < argc; a++) {
        Model model = Model(argv[a]);
        BlankShader shader(model);
        for (int i = 0; i < model.numFaces(); i++) {
            Triangle clip = { shader.vertex(i, 0),
                              shader.vertex(i, 1),
                              shader.vertex(i, 2) };
            rasterize(clip, trash, shader);
        }
    }
    
    TGAImage lightbuffer(s_width, s_height, TGAImage::GRAYSCALE);
    for (int x = 0; x < s_width; x++)
        for (int y = 0; y < s_height; y++)
                lightbuffer.set(x, y, {(unsigned char)(std::min(255*zbuffer[x+s_width*y],255.))});
    lightbuffer.write_tga_file("lightbuffer.tga");

    std::vector<double> light_zbuffer = zbuffer;
    matrix<4,4,double> N = Viewport * Perspective * ModelView;

    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width,height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209});

    for (int a = 1; a < argc; a++) {
        Model model = Model(argv[a]);
        PhongShader_nm shader(model, sun, light_zbuffer, s_width, s_height, N);
        for (int i = 0; i < model.numFaces(); i++) {
            Triangle clip = { shader.vertex(i, 0),
                              shader.vertex(i, 1),
                              shader.vertex(i, 2) };
            rasterize(clip, framebuffer, shader);
        }
    }

    if (edge_detection) {
        double threshold = .15;
        int sobel_x[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}}; 
        int sobel_y[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}}; 
        for (int x = 1; x < framebuffer.width() - 1; x++) {
            for (int y = 1; y < framebuffer.height() - 1; y++) {
                vec2 sum;
                for (int i = -1; i <= 1; i++) 
                    for (int j = -1; j <= 1; j++) 
                        sum = sum + vec2{
                            sobel_x[j + 1][i + 1] * zbuffer[x+i + (y+j)*framebuffer.width()],
                            sobel_y[j + 1][i + 1] * zbuffer[x+i + (y+j)*framebuffer.width()]
                        };
                if (norm(sum) > threshold) framebuffer.set(x,y,{0,0,0});
            }
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