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
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::string str;
            int f, cnt = 0;
            iss >> trash;
            while (iss >> str) {
                facet_vrt.push_back(stoi(str.substr(0, str.find("/"))) - 1);
                cnt++;
            }
            if (3 != cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            } 
        }
    }
    std::cerr << "# v# " << numVerts() << " f# "  << numFaces() << std::endl;
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