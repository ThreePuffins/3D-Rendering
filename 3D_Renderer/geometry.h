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
    vec<2, double> xy() const {return {x,y};}
    vec<3, double> xyz() const {return {x,y,z};}
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

inline vec3 cross(const vec3& v1, const vec3& v2) {
    return {v1.y*v2.z-v1.z*v2.y,v1.z*v2.x-v1.x*v2.z,v1.x*v2.y-v1.y*v2.x};
}

template<int n, typename T> struct dt; //used for recursive determinant calculation

template<int nrows, int ncols, typename T> struct matrix {
    vec<ncols, T> rows[nrows] = {{}};
    vec<ncols, T>& operator[](const int i) { assert(i>=0 && i<nrows); return rows[i]; }
    const vec<ncols, T>&  operator[](const int i) const { assert(i>=0 && i<nrows); return rows[i]; }
    T det() const {
        assert(nrows==ncols);
        return dt<nrows, T>::det(*this);
    }

    // https://www.cuemath.com/algebra/cofactor-matrix/
    T cofactor(const int row,const int col) const {
        matrix<nrows-1,ncols-1,T> sub;
        for (int i=0;i<nrows-1;i++)
            for (int j=0;j<ncols-1;j++) 
                sub[i][j]=rows[i+int(i>=row)][j+int(j>=col)];
        return sub.det() * ((row+col)%2 ? -1 : 1);
    }

    matrix<ncols, nrows, T> transpose() {
        assert(nrows==ncols);
        matrix<ncols, nrows, T> ret;
        for (int i=0;i<nrows;i++)
            for (int j=0;j<nrows;j++) 
                ret[i][j] = rows[j][i];
        return ret;
    }

    //https://www.mathsisfun.com/algebra/matrix-inverse-minors-cofactors-adjugate.html
    //shows how to inverse, but the last step is to transpose so may as well define a function for all
    //the previous work just in case it's necessary to use it
    matrix<ncols,nrows, T> invertTransposed() {
        assert(nrows==ncols);
        matrix<ncols, nrows, T> adjugate;
        for (int i=0;i<nrows;i++)
            for (int j=0;j<ncols;j++) 
                adjugate[i][j] = cofactor(i,j);
        // adjugate[0]*rows[0] is the determinant bc its matrix of minors * the original top row
        return adjugate/(adjugate[0]*rows[0]);
    }

    matrix<ncols,nrows, T> invert() {
        assert(nrows==ncols);
        return invertTransposed().transpose();
    }
};

template<int nrows,int ncols, typename T> vec<nrows, T> operator*(const matrix<nrows,ncols, T>& m, const vec<ncols, T>& v) {
    vec<nrows, T> ret;
    for (int i=0; i<nrows; i++) ret[i]=m[i]*v;
    return ret;
}

template<int nrows,int ncols, typename T> vec<ncols, T> operator*(const vec<nrows, T>& v, const matrix<nrows,ncols, T>& m) {
    return (matrix<1,nrows, T>{{v}}*m)[0];
}

template<int R1, int C1, int C2, typename T> matrix<R1, C2, T> 
    operator*(const matrix<R1, C1, T>& m1, const matrix<C1, C2, T>& m2) {
    matrix<R1, C2, T> ret;
    for (int r = 0; r < R1; r++)
        for (int c1 = 0; c1 < C1; c1++)
            for (int c2 = 0; c2 < C2; c2++) {ret[r][c2] += m1[r][c1] * m2[c1][c2];}
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
    for (int i = 0; i < nrows; i++) {ret[i] = m1[i]/val;}
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

template<int n, typename T>struct dt {
    static double det(const matrix<n,n, double>& src) {
        T ret;
        for (int i=0;i<n;i++)
            ret+=src[0][i]*src.cofactor(0,i);
        return ret;
    }
};

template<> struct dt<1, double> {
    static double det(const matrix<1,1, double>& src) {
        return src[0][0];
    }
};