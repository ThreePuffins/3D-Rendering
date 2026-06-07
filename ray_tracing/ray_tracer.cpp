
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