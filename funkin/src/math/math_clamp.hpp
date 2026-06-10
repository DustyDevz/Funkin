// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <cmath>

namespace Funkin::Math {
    inline float clamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    inline float clamp01(float v) {
        return clamp(v, 0.f, 1.f);
    }

    inline float remap(float v, float inLo, float inHi, float outLo, float outHi) {
        return outLo + (v - inLo) / (inHi - inLo) * (outHi - outLo);
    }

    inline float wrap(float v, float lo, float hi) {
        float range = hi - lo;
        return range == 0.f ? lo : v - range * std::floor((v - lo) / range);
    }
}