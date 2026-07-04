#include "geometry.h"


int main(int argc, char** argv) {

    vec3 a = {4,-2,1};

    matrix<3,3, double> mat = {{{6,2,4},{-1,4,3},{-2,9,3}}};

    vec3 r = mat * a;

    for (int i=0;i<3;i++)
            std::cerr << r[i] << std::endl;
}

// checked off geometry.h:
// matrix * matrix multiplication
// determinant
// cross product
// normalisation/norm
// vector matrix multiplication is fine