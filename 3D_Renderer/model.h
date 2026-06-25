#include <vector>
#include "geometry.h"


class Model {
    std::vector<vec3> verts = {};
    std::vector<int> facet_vrt = {};
public:
    Model(const std::string filename);
    int numVerts() const;
    int numFaces() const;
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int ivert) const;
};
