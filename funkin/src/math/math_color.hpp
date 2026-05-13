// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Math {
    struct Color32;
    struct Color {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        Color() = default;
        constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
        explicit Color(const Color32& c);

        Color  operator*(float s)        const { return { r*s, g*s, b*s, a*s }; }
        Color  operator*(const Color& o) const { return { r*o.r, g*o.g, b*o.b, a*o.a }; }
        Color  operator+(const Color& o) const { return { r+o.r, g+o.g, b+o.b, a+o.a }; }
        bool   operator==(const Color& o) const { return r==o.r && g==o.g && b==o.b && a==o.a; }
        bool   operator!=(const Color& o) const { return !(*this == o); }

        Color withAlpha(float newAlpha)  const { return { r, g, b, newAlpha }; }
        Color clamped()                  const {
            return {
                std::clamp(r, 0.0f, 1.0f),
                std::clamp(g, 0.0f, 1.0f),
                std::clamp(b, 0.0f, 1.0f),
                std::clamp(a, 0.0f, 1.0f)
            };
        }

        static Color lerp(const Color& a, const Color& b, float t) {
            return {
                a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t,
                a.a + (b.a - a.a) * t
            };
        }

        static constexpr Color white()       { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
        static constexpr Color black()       { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
        static constexpr Color red()         { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
        static constexpr Color green()       { return { 0.0f, 1.0f, 0.0f, 1.0f }; }
        static constexpr Color blue()        { return { 0.0f, 0.0f, 1.0f, 1.0f }; }
        static constexpr Color yellow()      { return { 1.0f, 1.0f, 0.0f, 1.0f }; }
        static constexpr Color cyan()        { return { 0.0f, 1.0f, 1.0f, 1.0f }; }
        static constexpr Color magenta()     { return { 1.0f, 0.0f, 1.0f, 1.0f }; }
        static constexpr Color transparent() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
        static constexpr Color gray()        { return { 0.5f, 0.5f, 0.5f, 1.0f }; }
    };

    struct Color32 {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;
        uint8_t a = 255;

        Color32() = default;
        constexpr Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
        explicit Color32(const Color& c)
            : r(static_cast<uint8_t>(std::clamp(c.r, 0.0f, 1.0f) * 255.0f))
            , g(static_cast<uint8_t>(std::clamp(c.g, 0.0f, 1.0f) * 255.0f))
            , b(static_cast<uint8_t>(std::clamp(c.b, 0.0f, 1.0f) * 255.0f))
            , a(static_cast<uint8_t>(std::clamp(c.a, 0.0f, 1.0f) * 255.0f))
        {}

        bool operator==(const Color32& o) const { return r==o.r && g==o.g && b==o.b && a==o.a; }
        bool operator!=(const Color32& o) const { return !(*this == o); }

        uint32_t packed() const {
            return (static_cast<uint32_t>(a) << 24)
                | (static_cast<uint32_t>(b) << 16)
                | (static_cast<uint32_t>(g) <<  8)
                | (static_cast<uint32_t>(r));
        }
    };

    inline Color::Color(const Color32& c)
        : r(c.r / 255.0f)
        , g(c.g / 255.0f)
        , b(c.b / 255.0f)
        , a(c.a / 255.0f)
    {}

    inline Color operator*(float s, const Color& c) { return c * s; }
}
