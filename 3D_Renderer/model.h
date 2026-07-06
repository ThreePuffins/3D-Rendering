#include <vector>
#include "geometry.h"


class Model {
    std::vector<vec3> verts = {};
    std::vector<vec3> vert_norms = {};
    std::vector<int> facet_vrt = {};
    std::vector<int> facet_nrm = {};
public:
    Model(const std::string filename);
    int numVerts() const;
    int numVertNormals() const;
    int numFaces() const;
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int ivert) const;
    vec3 vertNormal(const int iface, const int ivert) const;
};
