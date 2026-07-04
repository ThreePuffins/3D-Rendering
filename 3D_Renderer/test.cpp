#include "geometry.h"


int main(int argc, char** argv) {

    vec3 a = {2,3,4};
    vec3 b = {5,6,7};
    vec3 c = cross(a,b);

    for (int i=0;i<3;i++) {
        std::cerr << c[i] << std::endl;
    }
}

// checked off geometry.h:
// matrix * matrix multiplication
// determinant
// cross product (checked it twice)
// normalisation/norm
// vector matrix multiplication is fine
// IT WAS THE COFACTOR FUUUUUHHH