// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <SDL3/SDL.h>
#include <imgui.h>

namespace Funkin::ImGui_SDL3 {
    bool init(SDL_Window* window);
    void shutdown();
    void newFrame();
    bool processEvent(const SDL_Event& event);
}