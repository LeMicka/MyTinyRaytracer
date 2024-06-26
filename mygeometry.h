#ifndef MYGEOMETRY_H
#define MYGEOMETRY_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <cmath>

template<typename T>
class Vec2
{
public:
    Vec2() : x(0), y(0) {}
    Vec2(T xx) : x(xx), y(xx) {}
    Vec2(T xx, T yy) : x(xx), y(yy) {}
    Vec2 operator + (const Vec2 &v) const
    { return Vec2(x + v.x, y + v.y); }
    Vec2 operator / (const T &r) const
    { return Vec2(x / r, y / r); }
    Vec2 operator * (const T &r) const
    { return Vec2(x * r, y * r); }
    Vec2& operator /= (const T &r)
    { x /= r, y /= r; return *this; }
    Vec2& operator *= (const T &r)
    { x *= r, y *= r; return *this; }
    friend std::ostream& operator << (std::ostream &s, const Vec2<T> &v)
    {
        return s << '[' << v.x << ' ' << v.y << ']';
    }
    friend Vec2 operator * (const T &r, const Vec2<T> &v)
    { return Vec2(v.x * r, v.y * r); }

    const T& operator [] (uint8_t i) const { return (&x)[i]; }
    T& operator [] (uint8_t i) { return (&x)[i]; }
    
    T x, y;
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;

//[comment]
// Implementation of a generic vector class - it will be used to deal with 3D points, vectors and normals.
// The class is implemented as a template. While it may complicate the code a bit, it gives us
// the flexibility later, to specialize the type of the coordinates into anything we want.
// For example: Vec3f if we want the coordinates to be floats or Vec3i if we want the coordinates to be integers.
//
// Vec3 is a standard/common way of naming vectors, points, etc. The OpenEXR and Autodesk libraries
// use this convention for instance.
//[/comment]
template<typename T>
class Vec3
{
public:
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Vec3 operator + (const Vec3 &v) const
    { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator - (const Vec3 &v) const
    { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator - () const
    { return Vec3(-x, -y, -z); }
    Vec3 operator * (const T &r) const
    { return Vec3(x * r, y * r, z * r); }
    Vec3 operator * (const Vec3 &v) const
    { return Vec3(x * v.x, y * v.y, z * v.z); }
    T dotProduct(const Vec3<T> &v) const
    { return x * v.x + y * v.y + z * v.z; }
    Vec3& operator /= (const T &r)
    { x /= r, y /= r, z /= r; return *this; }
    Vec3& operator *= (const T &r)
    { x *= r, y *= r, z *= r; return *this; }
    Vec3 crossProduct(const Vec3<T> &v) const
    { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    T norm() const
    { return x * x + y * y + z * z; }
    T length() const
    { return sqrt(norm()); }
    //[comment]
    // The next two operators are sometimes called access operators or
    // accessors. The Vec coordinates can be accessed that way v[0], v[1], v[2],
    // rather than using the more traditional form v.x, v.y, v.z. This useful
    // when vectors are used in loops: the coordinates can be accessed with the
    // loop index (e.g. v[i]).
    //[/comment]
    const T& operator [] (uint8_t i) const { return (&x)[i]; }
    T& operator [] (uint8_t i) { return (&x)[i]; }
    Vec3& normalize()
    {
        T n = norm();
        if (n > 0) {
            T factor = 1 / sqrt(n);
            x *= factor, y *= factor, z *= factor;
        }
        
        return *this;
    }

    

    friend Vec3 operator * (const T &r, const Vec3 &v)
    { return Vec3<T>(v.x * r, v.y * r, v.z * r); }
    friend Vec3 operator / (const T &r, const Vec3 &v)
    { return Vec3<T>(r / v.x, r / v.y, r / v.z); }

    friend std::ostream& operator << (std::ostream &s, const Vec3<T> &v)
    {
        return s << '[' << v.x << ' ' << v.y << ' ' << v.z << ']';
    }
    
    T x, y, z;
};

//[comment]
// Now you can specialize the class. We are just showing two examples here. In your code
// you can declare a vector either that way: Vec3<float> a, or that way: Vec3f a
//[/comment]
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;




template<typename T>
class Vec4
{
public:
    Vec4() : x(T(0)), y(T(0)), z(T(0)), w(T(0)) {}
    Vec4(T xx) : x(xx), y(xx), z(xx), w(xx){}
    Vec4(T xx, T yy, T zz, T ww) : x(xx), y(yy), z(zz) , w(ww) {}
    Vec4 operator + (const Vec4 &v) const
    { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vec4 operator - (const Vec4 &v) const
    { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vec4 operator - () const
    { return Vec4(-x, -y, -z, -w); }
    Vec4 operator * (const T &r) const
    { return Vec4(x * r, y * r, z * r, w * r); }
    Vec4 operator * (const Vec4 &v) const
    { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    T dotProduct(const Vec4<T> &v) const
    { return x * v.x + y * v.y + z * v.z + w * v.w; }
    Vec4& operator /= (const T &r)
    { x /= r, y /= r, z /= r, w /= r; return *this; }
    Vec4& operator *= (const T &r)
    { x *= r, y *= r, z *= r, w *= r; return *this; }
    T norm() const
    { return x * x + y * y + z * z + w * w; }
    T length() const
    { return sqrt(norm()); }
    //[comment]
    // The next two operators are sometimes called access operators or
    // accessors. The Vec coordinates can be accessed that way v[0], v[1], v[2],
    // rather than using the more traditional form v.x, v.y, v.z. This useful
    // when vectors are used in loops: the coordinates can be accessed with the
    // loop index (e.g. v[i]).
    //[/comment]
    const T& operator [] (uint8_t i) const { return (&x)[i]; }
    T& operator [] (uint8_t i) { return (&x)[i]; }
    Vec4& normalize()
    {
        T n = norm();
        if (n > 0) {
            T factor = 1 / sqrt(n);
            x *= factor, y *= factor, z *= factor, w *= factor;
        }
        
        return *this;
    }

    

    friend Vec4 operator * (const T &r, const Vec4 &v)
    { return Vec4<T>(v.x * r, v.y * r, v.z * r, v.w * r); }
    friend Vec4 operator / (const T &r, const Vec4 &v)
    { return Vec4<T>(r / v.x, r / v.y, r / v.z, r / v.w); }

    friend std::ostream& operator << (std::ostream &s, const Vec4<T> &v)
    {
        return s << '[' << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w << ']';
    }
    
    T x, y, z, w;
};

//[comment]
// Now you can specialize the class. We are just showing two examples here. In your code
// you can declare a vector either that way: Vec4<float> a, or that way: Vec4f a
//[/comment]
typedef Vec4<float> Vec4f;
typedef Vec4<int> Vec4i;

#endif