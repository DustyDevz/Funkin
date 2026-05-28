// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <imgui.h>

namespace Funkin::ImGui_BGFX {
    bool init(bgfx::ViewId viewId = 255);
    void shutdown();
    void newFrame();
    void render(ImDrawData* drawData);
}