// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Math {
    struct Vec2 {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        explicit constexpr Vec2(float s)         : x(s),   y(s)   {}
        constexpr Vec2(float x, float y)         : x(x),   y(y)   {}

        Vec2  operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
        Vec2  operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }
        Vec2  operator*(float s)       const { return { x * s,   y * s   }; }
        Vec2  operator/(float s)       const { return { x / s,   y / s   }; }
        Vec2  operator-()              const { return { -x, -y }; }

        Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
        Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
        Vec2& operator*=(float s)       { x *= s;   y *= s;   return *this; }
        Vec2& operator/=(float s)       { x /= s;   y /= s;   return *this; }

        bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vec2& o) const { return !(*this == o); }

        float lengthSq()  const { return x * x + y * y; }
        float length()    const { return std::sqrtf(lengthSq()); }
        Vec2  normalized() const { float l = length(); return l > 0.0f ? *this / l : Vec2{}; }
        float dot(const Vec2& o) const { return x * o.x + y * o.y; }

        static Vec2 lerp(const Vec2& a, const Vec2& b, float t) { return a + (b - a) * t; }
        static Vec2 min(const Vec2& a, const Vec2& b) { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }
        static Vec2 max(const Vec2& a, const Vec2& b) { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

        static constexpr Vec2 zero()  { return { 0.0f,  0.0f  }; }
        static constexpr Vec2 one()   { return { 1.0f,  1.0f  }; }
        static constexpr Vec2 up()    { return { 0.0f, -1.0f  }; }
        static constexpr Vec2 down()  { return { 0.0f,  1.0f  }; }
        static constexpr Vec2 left()  { return {-1.0f,  0.0f  }; }
        static constexpr Vec2 right() { return { 1.0f,  0.0f  }; }
    };

    inline Vec2 operator*(float s, const Vec2& v) { return v * s; }
}
