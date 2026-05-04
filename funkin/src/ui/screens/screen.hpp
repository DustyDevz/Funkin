// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <math/vec2.hpp>
#include <cstdint>

namespace Funkin::UI {
    class Screen {
    public:
        virtual ~Screen() = default;

        virtual void init()                                = 0;
        virtual void update(Vec2 mousePos, bool mouseDown) = 0;
        virtual void draw()                                = 0;
        virtual void shutdown()                            = 0;
        virtual void onResize(uint32_t w, uint32_t h)      = 0;
    };
}