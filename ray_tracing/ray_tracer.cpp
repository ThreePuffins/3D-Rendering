
#include <cmath>
#include <vector>
#include <iostream>

#define MAX_RAY_DEPTH 10
#define AIR_REFRACTION_INDEX 1.00027717

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
    Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }

    Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }

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
    float transparency, refractIndex, reflection;
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &refl = 0,
        const float &transp = 0,
        const float &refrIndex = 0, //TODO: make transparency and all related variables a struct or similar
        const Vec3f &ec = 0) :
        centre(c), radius(r), radius2(r * r), surfaceCol(sc), emissionCol(ec),
        transparency(transp), refractIndex(refrIndex), reflection(refl)
    { /* empty */ }
    
    // Compute a ray-sphere intersection using the geometric solution
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
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


//returns a mixed value of a and b according to mix weighting, determining b's weighting
float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

// 
Vec3f trace(
    const Vec3f &rayorig,
    const Vec3f &rayDir,
    const std::vector<Sphere> &spheres,
    const float &refractionIndex,
    const int &depth
) 
{
    // TODO: figure out handling raydir automatic normalisation

    const Vec3f bg_col = Vec3f(1,1,1);

    const Sphere* closestSphere = NULL;
    float closest = INFINITY;
    for (int i = 0; i < spheres.size(); i++) {
        float t0 = INFINITY, t1 = INFINITY;
        if (spheres[i].intersect(rayorig, rayDir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < closest) {
                closest = t0;
                closestSphere = &spheres[i];
            }
        }
    }

    if (!closestSphere) return bg_col;

    float bias = 1e-4;

    Vec3f pHit = rayorig + (rayDir * closest);
    Vec3f nHit = (pHit - closestSphere->centre);
    nHit.normalize();

    bool inside = false;
    if (rayDir.dotProduct(nHit) > 0) nHit = -nHit, inside = true;

    Vec3f surfaceCol = 0;

    // reflection
    if ((closestSphere->reflection > 0 || closestSphere->transparency > 0) && depth < MAX_RAY_DEPTH) {
        
        Vec3f reflectDir = rayDir - (nHit * (nHit.dotProduct(rayDir))) * 2;
        reflectDir.normalize();
        // TODO: figure out if inverse square law is necessary
        Vec3f reflectionCol = trace(pHit, reflectDir, spheres, refractionIndex, depth + 1);
        
        Vec3f refractionCol = 0;
        if (closestSphere->transparency) {
            float refractionRatio = closestSphere->refractIndex / refractionIndex;
            if (!inside) refractionRatio = 1 / refractionRatio;
            // no division by magnitude bc they're normalised already
            float cosTheta = -nHit.dotProduct(rayDir);
            Vec3f refractDirPerp = (rayDir - nHit * cosTheta) * refractionRatio;
            Vec3f refractDirPar = nHit * -sqrt(abs(1 - refractDirPerp.length_sq()));
            Vec3f refractDir = refractDirPerp + refractDirPar;
            refractDir.normalize();
            // TODO: figure out checking the new refraction index
            float newRefraction = (!inside) ? closestSphere->refractIndex : AIR_REFRACTION_INDEX;
            refractionCol = trace(pHit + refractDir * bias, refractDir, spheres, newRefraction, depth + 1);
        }
        float facingRatio = -rayDir.dotProduct(nHit);
        // this is a loose estimation of the real effect
        float fresnelEffect = mix(pow(1 - facingRatio, 4), 1, 0.5);

        surfaceCol = (
            reflectionCol * fresnelEffect +
            refractionCol * (1 - fresnelEffect) * closestSphere->transparency) * closestSphere->surfaceCol;
        //surfaceCol = (reflectionCol) * closestSphere->surfaceCol;
    }
    // diffuse
    else {
        for (int i = 0; i < spheres.size(); i++) {
            if (spheres[i].emissionCol.length_sq() < 0.001f) continue;
            int transmission = 1;
            Vec3f lightDisplacement = spheres[i].centre - pHit;
            Vec3f lightDir = (spheres[i].centre - pHit).normalize();
            for (int j = 0; j < spheres.size(); j++) {
                if (i != j) {
                    float t0, t1;
                    if (spheres[j].intersect(pHit + nHit * bias, lightDir, t0, t1)) {
                        if (t0 * t0 > lightDisplacement.length_sq()) continue;
                        transmission = 0;
                        break;
                    }
                }
            }
            // Distance between pHit and light
            // float pHitLightDist = lightDisplacement.length() - closestSphere->radius
            //                  - spheres[i].radius;
            // float inverseSqCoeff = (1 / (4 * M_PI * pHitLightDist * pHitLightDist));
            // TODO: figure out if inverse square law is necessary

            surfaceCol += (closestSphere->surfaceCol * transmission *
                spheres[i].emissionCol *
                std::max(float(0), nHit.dotProduct(lightDir)));
        }
    }
    return surfaceCol + closestSphere->emissionCol;
}

void render(const std::vector<Sphere> &spheres)
{
    unsigned width = 1024, height = 1024;
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 60, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++, pixel++) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, 1);
            raydir.normalize();
            *pixel = trace(Vec3f(0), raydir, spheres, AIR_REFRACTION_INDEX, 0);
        }
    }
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";
    for (int i = 0; i < height * width; i++) {
        std::cout << (int)(std::min(float(1), image[i].x) * 255) << ' ' << 
                     (int)(std::min(float(1), image[i].y) * 255) << ' ' << 
                     (int)(std::min(float(1), image[i].z) * 255) << '\n';
    }
}

int main(int argc, char **argv)
{
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f( 0.0, -10004, 20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
    spheres.push_back(Sphere(Vec3f( 0.0, 0, 20), 4, Vec3f(1.00, 0.32, 0.36), 0, 1.0, 1.5));
    spheres.push_back(Sphere(Vec3f( 5.0, -1, 15), 3, Vec3f(0.90, 0.76, 0.46), 1, 0.0, 1.5));
    spheres.push_back(Sphere(Vec3f( 5.0, 0, 25), 4, Vec3f(0.65, 0.77, 0.97), 1, 0.0, 1.5));
    spheres.push_back(Sphere(Vec3f(-5.5, 0, 15), 4, Vec3f(0.90, 0.90, 0.90), 1, 0.0, 1.5));
    spheres.push_back(Sphere(Vec3f( 0.0, 10026, 20), 10000, Vec3f(1, 1, 1), 1, 0.0));
    

    // lights
    spheres.push_back(Sphere(Vec3f( -7, 20, 30), 0.4, Vec3f(0.00, 0.00, 0.00), 0, 0, 0.0, Vec3f(4,0,2)));
    spheres.push_back(Sphere(Vec3f( 7, 20, 30), 0.4, Vec3f(0.00, 0.00, 0.00), 0, 0, 0.0, Vec3f(0,4,2)));
    
    render(spheres);
    
    return 0;
}