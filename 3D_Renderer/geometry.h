#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n, typename T> struct vec {
    T data[n] = {0};
    T& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    T  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
};

template<int n, typename T> double operator*(const vec<n, T>& vl, const vec<n, T>& vr) {
    double sum = 0;
    for (int i=0; i<n; i++) sum += vl[i] * vr[i];
    return sum;
}

template<int n, typename T> std::ostream& operator<<(std::ostream& out, const vec<n, T>& v) {
    for (int i=0; i<n; i++) out << v[i] << " ";
    return out;
}

template<int n, typename T> vec<n, T> operator+(const vec<n, T>& vl, const vec<n, T>& vr) {
    vec<n,T> v = vl;
    for (int i=0; i<n; i++) v[i] += vr[i];
    return v;
}

template<int n, typename T> vec<n, T> operator-(const vec<n, T>& vl, const vec<n, T>& vr) {
    vec<n,T> v = vl;
    for (int i=0; i<n; i++) v[i] -= vr[i];
    return v;
}

template<int n, typename T> vec<n, T> operator*(const vec<n, T>& vl, const T s) {
    vec<n,T> v = vl;
    for (int i=0; i<n; i++) v[i] *= s;
    return v;
}

template<int n, typename T> vec<n, T> operator*(const T s, const vec<n, T>& vl) {
    return vl * s;
}

template<int n, typename T> vec<n, T> operator/(const vec<n, T>& vl, const T s) {
    vec<n,T> v = vl;
    for (int i=0; i<n; i++) v[i] /= s;
    return v;
}

template<> struct vec<2, double> {
    double x = 0, y = 0;
    double& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    double  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }
};

template<> struct vec<3, double> {
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

template<> struct vec<4, double> {
    double x = 0, y = 0, z = 0, w = 0;
    double& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (i==2 ? z : w);}
    double  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (i==2 ? z : w);}
};

typedef vec<2, double> vec2;
typedef vec<3, double> vec3;
typedef vec<4, double> vec4;

template<int n, typename T> T norm(const vec<n, T>& v) {
    return (T) std::sqrt(v * v);
}

template<int n, typename T> vec<n, T> normalised(const vec<n, T>& v) {
    return v / norm(v);
}

template<int nrows, int ncols, typename T> struct matrix {
    vec<ncols, T> rows[nrows] = {{}};
    vec<ncols, T>& operator[](const int i) { assert(i>=0 && i<nrows); return rows[i]; }
    const vec<ncols, T>&  operator[](const int i) const { assert(i>=0 && i<nrows); return rows[i]; }
};

template<int nrows,int ncols, typename T> vec<nrows, T> operator*(const matrix<nrows,ncols, T>& m, const vec<ncols, T>& v) {
    vec<nrows, T> ret;
    for (int i=nrows; i--; ret[i]=m[i]*v);
    return ret;
}


template<int R1, int C1, int C2, typename T> matrix<R1, C2, T> 
    operator*(const matrix<R1, C1, T>& m1, const matrix<C1, C2, T>& m2) {
    matrix<R1, C2, T> ret;
    for (int r = 0; r < R1; r++)
        for (int c1 = 0; c1 < C1; c1++)
            for (int c2 = 0; c2 < C2; c2++) {ret[r][c1] += m1[r][c2] * m2[c2][c1];}
    return ret;
}

template<int nrows, int ncols, typename T> matrix<nrows, ncols, T> operator*(const matrix<nrows, ncols, T>& m1, T val) {
    matrix<nrows, ncols, T> ret = m1;
    for (int r = 0; r < nrows; r++)
        for (int c = 0; c < ncols; c++) {ret[nrows][ncols] *= val;}
    return ret;
}

template<int nrows, int ncols, typename T> matrix<nrows, ncols, T> operator*(T val, const matrix<nrows, ncols, T>& m1) {
    return m1 * val;
}

template<int nrows, int ncols, typename T> matrix<nrows, ncols, T> operator/(const matrix<nrows, ncols, T>& m1, T val) {
    matrix<nrows, ncols, T> ret = m1;
    for (int r = 0; r < nrows; r++)
        for (int c = 0; c < ncols; c++) {ret[nrows][ncols] /= val;}
    return ret;
}

template<int nrows, int ncols, typename T> matrix<nrows, ncols, T> operator+(const matrix<nrows, ncols, T>& m1, const matrix<nrows, ncols, T>& m2) {
    matrix<nrows, ncols, T> ret = m1;
    for (int r = 0; r < nrows; r++)
        for (int c = 0; c < ncols; c++) {ret[nrows][ncols] += m2[nrows][ncols];}
    return ret;
}

template<int nrows, int ncols, typename T> matrix<nrows, ncols, T> operator-(const matrix<nrows, ncols, T>& m1, const matrix<nrows, ncols, T>& m2) {
    matrix<nrows, ncols, T> ret = m1;
    for (int r = 0; r < nrows; r++)
        for (int c = 0; c < ncols; c++) {ret[nrows][ncols] -= m2[nrows][ncols];}
    return ret;
}