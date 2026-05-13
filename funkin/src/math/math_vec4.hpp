// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "math_vec3.hpp"

namespace Funkin::Math {
    struct Vec4 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        Vec4() = default;
        explicit constexpr Vec4(float s)                        : x(s),   y(s),   z(s),   w(s)   {}
        constexpr Vec4(float x, float y, float z, float w)      : x(x),   y(y),   z(z),   w(w)   {}
        constexpr Vec4(const Vec3& v, float w)                  : x(v.x), y(v.y), z(v.z), w(w)   {}

        Vec4  operator+(const Vec4& o) const { return { x+o.x, y+o.y, z+o.z, w+o.w }; }
        Vec4  operator-(const Vec4& o) const { return { x-o.x, y-o.y, z-o.z, w-o.w }; }
        Vec4  operator*(float s)       const { return { x*s,   y*s,   z*s,   w*s   }; }
        Vec4  operator/(float s)       const { return { x/s,   y/s,   z/s,   w/s   }; }
        Vec4  operator-()              const { return { -x, -y, -z, -w }; }

        Vec4& operator+=(const Vec4& o) { x+=o.x; y+=o.y; z+=o.z; w+=o.w; return *this; }
        Vec4& operator-=(const Vec4& o) { x-=o.x; y-=o.y; z-=o.z; w-=o.w; return *this; }
        Vec4& operator*=(float s)       { x*=s;   y*=s;   z*=s;   w*=s;   return *this; }
        Vec4& operator/=(float s)       { x/=s;   y/=s;   z/=s;   w/=s;   return *this; }

        bool operator==(const Vec4& o) const { return x==o.x && y==o.y && z==o.z && w==o.w; }
        bool operator!=(const Vec4& o) const { return !(*this == o); }

        float dot(const Vec4& o) const { return x*o.x + y*o.y + z*o.z + w*o.w; }
        Vec3  xyz()              const { return { x, y, z }; }

        static Vec4 lerp(const Vec4& a, const Vec4& b, float t) { return a + (b - a) * t; }

        static constexpr Vec4 zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
        static constexpr Vec4 one()  { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
    };

    inline Vec4 operator*(float s, const Vec4& v) { return v * s; }
}