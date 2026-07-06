#include "bad_gl.h"
#include "model.h"

extern matrix<4,4,double> ModelView, Perspective;
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

struct PhongShader_nm : IShader {
    const Model &model;
    vec4 l;
    vec2 vert_uv[3];
    vec4 tri[3];
    vec4 vn[3];

    PhongShader_nm(const Model &m, const vec3 light) : model(m) {
        //TODO: make it a point light cuz that'd be cool 
        //directional light i think
        l = normalised((ModelView * vec4{light.x,light.y,light.z,1.})); 
    }

    virtual vec4 vertex(const int face, const int vert) {
        vert_uv[vert] = model.uv(face,vert);
        vec4 cam_pos = ModelView * model.vert(face,vert);
        tri[vert] = cam_pos;
        vn[vert] = ModelView.invertTransposed() * model.vertNormal(face,vert);
        vec4 n = model.vertNormal(face, vert);
        return Perspective * cam_pos;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
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
        double diffuse = .9*std::max(n * l,0.); // dot product is more efficient than cos
        // bc modelview makes the z axis parallel to the camera-eye vector, the dot product of the reflection and said vector is just the z component of r
        double specular= 2. * std::pow(std::max(r.z,0.),30);
        for (int c : {0,1,2}) {
            frag_col[c] = std::min<int>(255,frag_col[c]*(sample_uv(model.glow(),uv)[c]/255. 
                + ambient + diffuse + specular * (sample_uv(model.specular(),uv)[c]/255.)));
        }
        return {false, frag_col};
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
    vec3 light {2,2,2};

    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width,height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int a = 1; a < argc; a++) {
        Model model = Model(argv[a]);
        PhongShader_nm shader(model, light);
        for (int i = 0; i < model.numFaces(); i++) {
            Triangle clip = { shader.vertex(i, 0),
                              shader.vertex(i, 1),
                              shader.vertex(i, 2) };
            rasterize(clip, framebuffer, shader);
        }
    }

    // TGAImage depthbuffer(width, height, TGAImage::GRAYSCALE);
    // for (int x = 0; x < width; x++)
    //     for (int y = 0; y < height; y++)
    //             depthbuffer.set(x, y, {(unsigned char)(std::min(255*zbuffer[x+width*y],255.))});
    // depthbuffer.write_tga_file("depthbuffer.tga");
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}