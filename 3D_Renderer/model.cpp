#include "model.h"
#include <fstream>
#include <sstream>

Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename);
    if (in.fail()) return;
    std::string line;
    while(!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (line.compare(0, 1, "v")) {
            iss >> trash;
            vec3 v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts.push_back(v);
        }
        else if (line.compare(0, 1, "f")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            } 
        }
    }
}

int Model::numVerts() const {
    return verts.size();
}

int Model::numFaces() const {
    return facet_vrt.size() / 3;
}

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int ivert) const {
    return verts[facet_vrt[iface * 3 + ivert]];
}