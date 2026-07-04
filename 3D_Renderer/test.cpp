#include "geometry.h"


int main(int argc, char** argv) {

    vec3 a = {4,-2,1};

    matrix<2,2, double> mat = {{{4,7},{2,6}}};

    matrix<2,2, double> m1 = mat.invert();    

    for (int i=0;i<2;i++)
        for (int j=0;j<2;j++)
            std::cerr << m1[i][j] << std::endl;
}

// checked off geometry.h:
// matrix * matrix multiplication
// determinant
// cross product
// normalisation/norm
// vector matrix multiplication is fine