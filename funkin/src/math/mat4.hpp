// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "vec3.hpp"
#include "vec4.hpp"

namespace Funkin {

struct Mat4 {
    float m[16] = {};

    Mat4() = default;

    float& operator()(int row, int col)       { return m[row * 4 + col]; }
    float  operator()(int row, int col) const { return m[row * 4 + col]; }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                for (int k = 0; k < 4; ++k)
                    r(row, col) += (*this)(row, k) * o(k, col);
        return r;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            m[0]*v.x + m[1]*v.y + m[2]*v.z  + m[3]*v.w,
            m[4]*v.x + m[5]*v.y + m[6]*v.z  + m[7]*v.w,
            m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]*v.w,
            m[12]*v.x+ m[13]*v.y+ m[14]*v.z + m[15]*v.w
        };
    }

    Mat4 transposed() const {
        Mat4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r(i, j) = (*this)(j, i);
        return r;
    }

    static Mat4 identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    static Mat4 translation(const Vec3& t) {
        Mat4 r = identity();
        r(0,3) = t.x;
        r(1,3) = t.y;
        r(2,3) = t.z;
        return r;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 r = identity();
        r(0,0) = s.x;
        r(1,1) = s.y;
        r(2,2) = s.z;
        return r;
    }

    static Mat4 rotationX(float radians) {
        Mat4 r = identity();
        float c = std::cosf(radians), s = std::sinf(radians);
        r(1,1) =  c; r(1,2) = -s;
        r(2,1) =  s; r(2,2) =  c;
        return r;
    }

    static Mat4 rotationY(float radians) {
        Mat4 r = identity();
        float c = std::cosf(radians), s = std::sinf(radians);
        r(0,0) =  c; r(0,2) =  s;
        r(2,0) = -s; r(2,2) =  c;
        return r;
    }

    static Mat4 rotationZ(float radians) {
        Mat4 r = identity();
        float c = std::cosf(radians), s = std::sinf(radians);
        r(0,0) =  c; r(0,1) = -s;
        r(1,0) =  s; r(1,1) =  c;
        return r;
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Mat4 r;
        r(0,0) =  2.0f / (right - left);
        r(1,1) =  2.0f / (top - bottom);
        r(2,2) =  1.0f / (farZ - nearZ);
        r(0,3) = -(right + left)   / (right - left);
        r(1,3) = -(top   + bottom) / (top   - bottom);
        r(2,3) = -nearZ            / (farZ  - nearZ);
        r(3,3) =  1.0f;
        return r;
    }

    static Mat4 perspective(float fovYRadians, float aspect, float nearZ, float farZ) {
        Mat4 r;
        float f = 1.0f / std::tanf(fovYRadians * 0.5f);
        r(0,0) = f / aspect;
        r(1,1) = f;
        r(2,2) = farZ / (farZ - nearZ);
        r(2,3) = -nearZ * farZ / (farZ - nearZ);
        r(3,2) = 1.0f;
        return r;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);
        Mat4 m;
        m(0,0) =  r.x; m(0,1) =  r.y; m(0,2) =  r.z; m(0,3) = -r.dot(eye);
        m(1,0) =  u.x; m(1,1) =  u.y; m(1,2) =  u.z; m(1,3) = -u.dot(eye);
        m(2,0) =  f.x; m(2,1) =  f.y; m(2,2) =  f.z; m(2,3) = -f.dot(eye);
        m(3,3) = 1.0f;
        return m;
    }
};

}
