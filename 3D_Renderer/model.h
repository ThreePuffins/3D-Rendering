#include "geometry.h"
#include "tgaimage.h"


class Model {
    std::vector<vec4> verts = {};
    std::vector<vec4> vert_norms = {};
    std::vector<int> facet_vrt = {};
    std::vector<int> facet_nrm = {};
    std::vector<int> facet_tex = {};
    std::vector<vec2> tex = {};
    TGAImage normal_map = {};
    TGAImage specular_map = {};
    TGAImage diffuse_map = {};
public:
    Model(const std::string filename);
    int numVerts() const;
    int numVertNormals() const;
    int numFaces() const;
    vec4 vert(const int i) const;
    vec4 vert(const int iface, const int ivert) const;
    vec4 vertNormal(const int iface, const int ivert) const;
    vec4 normal(const vec2 &uv) const;
    const TGAImage& diffuse() const;
    const TGAImage& specular() const;
    vec2 uv(const int iface, const int ivert) const;
};
