#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <string>
#include "shared/log.hpp"
#include "input/input.hpp"

struct BgfxCallback : public bgfx::CallbackI {
    void fatal(const char* filePath, uint16_t line,
               bgfx::Fatal::Enum code, const char* str) override {
        LOG_CRIT("{}({}): {}", filePath, line, str);
    }

   void traceVargs(const char* filePath, uint16_t line,
                const char* format, va_list argList) override {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), format, argList);
        std::string_view sv = buf;
        while (!sv.empty() && (sv.back() == '\n' || sv.back() == '\r'))
            sv.remove_suffix(1);
        if (!sv.empty())
            LOG_PRINT("{}", sv);
    }
    
    void profilerBegin(const char*, uint32_t, const char*, uint16_t) override {}
    void profilerBeginLiteral(const char*, uint32_t, const char*, uint16_t) override {}
    void profilerEnd() override {}
    uint32_t cacheReadSize(uint64_t) override { return 0; }
    bool cacheRead(uint64_t, void*, uint32_t) override { return false; }
    void cacheWrite(uint64_t, const void*, uint32_t) override {}
    void screenShot(const char*, uint32_t, uint32_t, uint32_t,
                    bgfx::TextureFormat::Enum,
                    const void*, uint32_t, bool) override {}
    void captureBegin(uint32_t, uint32_t, uint32_t,
                      bgfx::TextureFormat::Enum, bool) override {}
    void captureEnd() override {}
    void captureFrame(const void*, uint32_t) override {}
};

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        LOG_ERR("SDL init failed: {}", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FNF CPP | initializing...",
        1280, 720,
        SDL_WINDOW_RESIZABLE);

    if (!window) {
        LOG_ERR("SDL window failed: {}", SDL_GetError());
        return 1;
    }

    LOG_PRINT("SDL window OK");

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props,
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

    LOG_PRINT("HWND = {}", hwnd);

    bgfx::PlatformData pd{};
    pd.nwh = hwnd;
    pd.ndt = nullptr;
    bgfx::setPlatformData(pd);

    bgfx::renderFrame();

    static BgfxCallback cb;

    bgfx::Init init{};
    init.callback          = &cb;
    init.platformData      = pd;
    init.resolution.width  = 1280;
    init.resolution.height = 720;
    init.resolution.reset  = BGFX_RESET_VSYNC;
    init.type              = bgfx::RendererType::Vulkan;

    LOG_PRINT("bgfx initializing...");

    if (!bgfx::init(init)) {
        LOG_CRIT("bgfx init failed");
        return 1;
    }

    const char* name = bgfx::getRendererName(bgfx::getRendererType());
    LOG_PRINT("bgfx using: {}", name);
    SDL_SetWindowTitle(window, (std::string("FNF [") + name + "]").c_str());

    bgfx::setViewClear(0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x1a1a1aff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 1280, 720);

    Funkin::Input::Input& input = Funkin::Input::Input::get();
    input.init();
    input.setWindow(window);

    // test binding
    input.bind("test", Funkin::Input::KeyCode::G);
    // debug binding
    input.bind("debug", Funkin::Input::KeyCode::F1);

    bool running = true;
    bool debug   = false;
    SDL_Event e;
    constexpr int TARGET_FPS = 9999999;
    constexpr double TARGET_MS = 1000.0 / TARGET_FPS;
    
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            
            input.handleSDLEvent(e);

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                uint32_t w = (uint32_t)e.window.data1;
                uint32_t h = (uint32_t)e.window.data2;
                bgfx::reset(w, h, BGFX_RESET_VSYNC);
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
            debug = !debug;
            LOG_PRINT("debug: {}", debug);

            if (debug) {
                bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
            } else {
                bgfx::setDebug(0);
            }
        }

        bgfx::touch(0);
        bgfx::frame();
    }

    input.shutdown();
    bgfx::shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}