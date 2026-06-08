
#include <cmath>
#include <vector>
#include <iostream>

template<typename T>
class Vec3 
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

    Vec3<T> operator * (const T &scalar) const { return Vec3<T>(x * scalar, y * scalar, z * scalar); }
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }

    Vec3& normalize()
    {
        T norm = length_sq();
        // can't normalise a zero vector
        if (norm > 0) {
            T invNorm = 1 / sqrt(norm);
            x *= invNorm, y *= invNorm, z *= invNorm;
        }
        return *this;
    }

    T dotProduct(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }

    T length_sq() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length_sq()); }

};

typedef Vec3<float> Vec3f;

class Sphere
{
public:
    Vec3f centre;
    float radius, radius2;
    Vec3f surfaceCol, emissionCol;
    float transparency, reflection;
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &refl = 0,
        const float &transp = 0,
        const Vec3f &ec = 0) :
        centre(c), radius(r), radius2(r * r), surfaceCol(sc), emissionCol(ec),
        transparency(transp), reflection(refl)
    { /* empty */ }
    
    //[comment]
    // Compute a ray-sphere intersection using the geometric solution
    //[/comment]
    bool intercept(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        Vec3f l = centre - rayorig;
        float tc = l.dotProduct(raydir);
        // points in wrong direction, exit
        if (tc < 0) return false;
        // use pythag to find square distance between sphere centre to closest ray point
        float d2 = l.dotProduct(l) - tc * tc;
        if (d2 > radius2) return false;
        // thc is dist from closest ray point to sphere interception point
        float thc = sqrt(radius2 - d2);
        t0 = tc - thc;
        t1 = tc + thc;
        
        return true;
    }
};

//[comment]
    // returns a mixed value of a and b according to mix weighting, determining b's weighting
//[/comment]
float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

//[comment]
    // 
//[/comment]
Vec3f trace(
    const Vec3f &rayorig,
    const Vec3f &raydir,
    const std::vector<Sphere> &spheres,
    const int &depth
) 
{
    return Vec3f(1);
}


void render(const std::vector<Sphere> &spheres)
{
    unsigned width = 640, height = 480;
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.0);

    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();
            *pixel = trace(Vec3f(0), raydir, spheres, 0);
        }
    }
    
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for (int i = 0; i < height * width; i++) {
        std::cout << (std::min(float(1), image[i].x) * 255) << ' ' << 
                     (std::min(float(1), image[i].y) * 255) << ' ' << 
                     (std::min(float(1), image[i].z) * 255) << '\n';
    }

}

int main(int argc, char **argv)
{
    std::vector<Sphere> spheres;

    render(spheres);
    
    return 0;
}