// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <string>
#include "shared/log.hpp"
#include "input/input.hpp"
#include "settings.hpp"
#include "app/debug.hpp"

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        LOG_ERR("SDL init failed: {}", SDL_GetError());
        return 1;
    }

    Funkin::Settings appSettings;
    SDL_Window* window = SDL_CreateWindow("FNF CPP | initializing...", appSettings.windowWidth, appSettings.windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOG_ERR("SDL window failed: {}", SDL_GetError());
        return 1;
    }

    LOG_PRINT("SDL window OK");

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LOG_PRINT("HWND = {}", hwnd);

    bgfx::PlatformData pd{};
    pd.nwh = hwnd;
    pd.ndt = nullptr;
    bgfx::setPlatformData(pd);

    bgfx::renderFrame();

    bgfx::Init init{};
    init.platformData      = pd;
    init.resolution.width  = appSettings.windowWidth;
    init.resolution.height = appSettings.windowHeight;
    init.resolution.reset  = BGFX_RESET_VSYNC;
    init.type              = bgfx::RendererType::Vulkan;

    LOG_PRINT("bgfx initializing...");

    if (!bgfx::init(init)) {
        LOG_CRIT("bgfx init failed");
        return 1;
    }

    Funkin::DebugManager::init(window, 255);

    const char* name = bgfx::getRendererName(bgfx::getRendererType());
    LOG_PRINT("bgfx using: {}", name);
    SDL_SetWindowTitle(window, (std::string("FNF [") + name + "]").c_str());

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, appSettings.windowWidth, appSettings.windowHeight);

    Funkin::Input::Input& input = Funkin::Input::Input::get();
    input.init();
    input.setWindow(window);
    input.bind("test", Funkin::Input::KeyCode::G);
    input.bind("debug", Funkin::Input::KeyCode::F1);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            
            input.handleSDLEvent(e);
            Funkin::DebugManager::handleEvent(e);

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                uint32_t w = (uint32_t)e.window.data1;
                uint32_t h = (uint32_t)e.window.data2;
                bgfx::reset(w, h, (appSettings.vsync == Funkin::Settings::VSyncMode::On) ? BGFX_RESET_VSYNC : 0);
                bgfx::setViewRect(0, 0, 0, (uint16_t)w, (uint16_t)h);
            }
        }

        input.update();

        if (input.justDown("test")) {
            uint64_t eventTime = input.getLastTimestamp("test");
            uint64_t now = input.getNow();
            LOG_PRINT("input latency: {:.4f} ms", (double)(now - eventTime) * 1e-6);
        }

        if (input.justDown("debug")) {
            Funkin::DebugManager::toggleDebugStats();
        }

        Funkin::DebugManager::beginFrame();
        Funkin::DebugManager::endFrame();

        bgfx::touch(0);
        bgfx::frame();
    }

    Funkin::DebugManager::shutdown();
    input.shutdown();
    bgfx::shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}