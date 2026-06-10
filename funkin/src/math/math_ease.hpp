// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <cmath>

namespace Funkin::Math {
    inline float easeLinear(float t)    { return t; }
    inline float easeQuadIn(float t)    { return t * t; }
    inline float easeQuadOut(float t)   { return t * (2.f - t); }
    inline float easeQuadInOut(float t) { return t < .5f ? 2.f*t*t : -1.f+(4.f-2.f*t)*t; }
    inline float easeCubicOut(float t)  { float f = t-1.f; return 1.f+f*f*f; }
    inline float easeSineOut(float t)   { return std::sinf(t * 1.5707963f); }
}