// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "core/engine.hpp"
#include <iostream>

#ifdef _WIN32
    #include <Windows.h>
    #include <cstdio>
        int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
    #else
        int main() {
    #endif

    Funkin::Core::EngineConfig cfg;
    cfg.title = "Funkin";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.vsync = true;

    auto& engine = Funkin::Core::Engine::get();
    engine.init(cfg);
    engine.run();
    engine.shutdown();

    return 0;
}