// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <cmath>

namespace Funkin::Math {
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    inline float damp(float a, float b, float lambda, float dt) {
        return lerp(a, b, 1.f - std::expf(-lambda * dt));
    }
}