// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

namespace Funkin::DebugManager {
    void init(SDL_Window* window, bgfx::ViewId viewId);
    void handleEvent(const SDL_Event& e);
    void beginFrame();
    void endFrame();
    void shutdown();
    void toggleDebugStats();
    bool isEnabled();
}