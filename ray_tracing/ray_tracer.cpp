
#include <cmath>

template<typename T>
class Vec3 
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

    Vec3<T> operator * (const T &scalar) const { return Vec3<T>(x * scalar, y * scalar, z * scalar)}
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z) }
    Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z) }

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

    T dotProduct(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z }

    T length_sq() const { return x * x + y * y + z * z}
    T length() const { return sqrt(length_sq())}

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



