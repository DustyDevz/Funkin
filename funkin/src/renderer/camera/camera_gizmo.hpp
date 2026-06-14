// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include "camera.hpp"
#include "assets/assets.hpp"
#include "renderer/sprite/sprite.hpp"

namespace Funkin::Renderer::Camera {
    class CameraGizmo {
    public:
        static void draw(
            uint16_t viewId,
            const GameCamera& cam,
            Funkin::Renderer::Sprite& iconSprite
        );
    };
}