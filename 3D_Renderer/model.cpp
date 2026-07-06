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
            vec4 v = {0,0,0,1};
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::string str;
            int f,t,n,cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            if (3 != cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            } 
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec4 vn;
            for (int i = 0; i < 3; i++) iss >> vn[i];
            vert_norms.push_back(normalised(vn));
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i = 0; i < 2; i++) iss >> uv[i];
            tex.push_back({uv.x,1-uv.y});
        }
    }
    std::cerr << "# v# " << numVerts() 
              << "# vn# " << numVertNormals()
              << " f# "  << numFaces() << std::endl;

    //lambda which adds the conventional suffix for various texture files (like _nm.tga) to the original file name (typically .obj)
    auto load_texture = [&filename](const std::string suffix, TGAImage &img) {
        size_t dot = filename.find_last_of(".");
        if (dot==std::string::npos) return; //if . is final char
        std::string texfile = filename.substr(0,dot) + suffix;
        //loads the tga file into img
        std::cerr << "texture file " << texfile << " loading " 
            << (img.read_tga_file(texfile.c_str()) ? "success" : "failed") << std::endl;
    };
    load_texture("_nm.tga", normal_map);
    load_texture("_diffuse.tga", diffuse_map);
    load_texture("_spec.tga", specular_map);
}

int Model::numVerts() const {
    return verts.size();
}

int Model::numFaces() const {
    return facet_vrt.size() / 3;
}

int Model::numVertNormals() const {
    return verts.size();
}

vec4 Model::vert(const int i) const {
    return verts[i];
}

vec4 Model::vert(const int iface, const int ivert) const {
    return verts[facet_vrt[iface * 3 + ivert]];
}

vec4 Model::vertNormal(const int iface, const int ivert) const {
    return vert_norms[facet_vrt[iface * 3 + ivert]];
}

vec4 Model::normal(const vec2 &uv) const {
    // TODO : understand why the domains seem to not match up between the vt uvs and the normal map
    TGAColor c = normal_map.get(uv[0] * normal_map.width(),uv[1] * normal_map.height());
    // tga uses bgra so reverses the channels, also maps the domains from [0,255] to [-1,1]
    return vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0}; 
}

const TGAImage& Model::diffuse() const {
    return diffuse_map;
}

const TGAImage& Model::specular() const {
    return specular_map;
}

vec2 Model::uv(const int iface, const int ivert) const {
    return tex[facet_tex[iface*3+ivert]];
}

