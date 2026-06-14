// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "camera_gizmo.hpp"
#include <debugdraw/debugdraw.h>
#include <bx/math.h>

namespace Funkin::Renderer::Camera {
    void CameraGizmo::draw(uint16_t viewId, const GameCamera &cam, Funkin::Renderer::Sprite &iconSprite) {
        auto fov = computeFov(cam.state(), cam.targetWidth, cam.targetHeight);

        DebugDrawEncoder dde;
        dde.begin(viewId);
        dde.setColor(0x4061afef);
        dde.setWireframe(true);

        bx::Aabb aabb;
        aabb.min = bx::Vec3(fov.left, fov.top, -1.f);
        aabb.max = bx::Vec3(fov.right, fov.bottom, 1.f);
        dde.draw(aabb);
        dde.end();

        iconSprite.x = cam.state().x;
        iconSprite.y = cam.state().y;
        iconSprite.viewId = viewId;
        iconSprite.draw();
    }
}
