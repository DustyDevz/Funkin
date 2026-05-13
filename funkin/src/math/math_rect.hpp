// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "math_vec2.hpp"

namespace Funkin::Math {
    struct Rect {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;

        Rect() = default;
        constexpr Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
        constexpr Rect(Vec2 position, Vec2 size)            : x(position.x), y(position.y), w(size.x), h(size.y) {}

        Vec2 position()    const { return { x, y }; }
        Vec2 size()        const { return { w, h }; }
        Vec2 center()      const { return { x + w * 0.5f, y + h * 0.5f }; }
        Vec2 topLeft()     const { return { x,     y     }; }
        Vec2 topRight()    const { return { x + w, y     }; }
        Vec2 bottomLeft()  const { return { x,     y + h }; }
        Vec2 bottomRight() const { return { x + w, y + h }; }

        float left()   const { return x; }
        float right()  const { return x + w; }
        float top()    const { return y; }
        float bottom() const { return y + h; }

        bool contains(Vec2 p)    const { return p.x >= x && p.x <= x+w && p.y >= y && p.y <= y+h; }
        bool contains(Rect r)    const { return r.x >= x && r.right() <= right() && r.y >= y && r.bottom() <= bottom(); }
        bool intersects(Rect r)  const { return x < r.right() && right() > r.x && y < r.bottom() && bottom() > r.y; }

        Rect expanded(float amount) const { return { x-amount, y-amount, w+amount*2.0f, h+amount*2.0f }; }
        Rect translated(Vec2 delta) const { return { x+delta.x, y+delta.y, w, h }; }
        Rect scaled(float s)        const { return { x*s, y*s, w*s, h*s }; }

        bool operator==(const Rect& o) const { return x==o.x && y==o.y && w==o.w && h==o.h; }
        bool operator!=(const Rect& o) const { return !(*this == o); }

        static Rect fromMinMax(Vec2 mn, Vec2 mx) { return { mn.x, mn.y, mx.x-mn.x, mx.y-mn.y }; }
        static constexpr Rect zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
    };
}
