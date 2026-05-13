// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Math {
    struct Vec3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vec3() = default;
        explicit constexpr Vec3(float s)                  : x(s),   y(s),   z(s)   {}
        constexpr Vec3(float x, float y, float z)         : x(x),   y(y),   z(z)   {}

        Vec3  operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        Vec3  operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        Vec3  operator*(float s)       const { return { x * s,   y * s,   z * s   }; }
        Vec3  operator/(float s)       const { return { x / s,   y / s,   z / s   }; }
        Vec3  operator-()              const { return { -x, -y, -z }; }

        Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
        Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
        Vec3& operator*=(float s)       { x *= s;   y *= s;   z *= s;   return *this; }
        Vec3& operator/=(float s)       { x /= s;   y /= s;   z /= s;   return *this; }

        bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
        bool operator!=(const Vec3& o) const { return !(*this == o); }

        float lengthSq()   const { return x * x + y * y + z * z; }
        float length()     const { return std::sqrtf(lengthSq()); }
        Vec3  normalized() const { float l = length(); return l > 0.0f ? *this / l : Vec3{}; }

        float dot(const Vec3& o)   const { return x * o.x + y * o.y + z * o.z; }
        Vec3  cross(const Vec3& o) const {
            return {
                y * o.z - z * o.y,
                z * o.x - x * o.z,
                x * o.y - y * o.x
            };
        }

        static Vec3 lerp(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }
        static Vec3 min(const Vec3& a, const Vec3& b) { return { std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z) }; }
        static Vec3 max(const Vec3& a, const Vec3& b) { return { std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z) }; }

        static constexpr Vec3 zero()    { return { 0.0f,  0.0f,  0.0f  }; }
        static constexpr Vec3 one()     { return { 1.0f,  1.0f,  1.0f  }; }
        static constexpr Vec3 up()      { return { 0.0f,  1.0f,  0.0f  }; }
        static constexpr Vec3 down()    { return { 0.0f, -1.0f,  0.0f  }; }
        static constexpr Vec3 left()    { return {-1.0f,  0.0f,  0.0f  }; }
        static constexpr Vec3 right()   { return { 1.0f,  0.0f,  0.0f  }; }
        static constexpr Vec3 forward() { return { 0.0f,  0.0f,  1.0f  }; }
        static constexpr Vec3 back()    { return { 0.0f,  0.0f, -1.0f  }; }
    };

    inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
}
