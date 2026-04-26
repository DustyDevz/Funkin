#include "engine.hpp"

#ifdef _WIN32
    #include <platform/window/win32_window.hpp>
    #include <renderer/dx12/renderer_dx12.hpp>
#endif

namespace Funkin::Core {
    Engine& Engine::get() {
        static Engine s;
        return s;
    }

    void Engine::init(const EngineConfig& cfg) {
        m_cfg = cfg;
        m_running = true;

    #ifdef _WIN32
        Platform::Win32_Window::get().init(cfg.title, cfg.width, cfg.height);
        Renderer::DX12::DX12Renderer::get().init(cfg.width, cfg.height, cfg.vsync);
    #endif
    }

    void Engine::run() {
        while (m_running) {
            #ifdef _WIN32
                if (!Platform::Win32_Window::get().pump()) {
                    quit();
                    break;
                }
                
                Renderer::DX12::DX12Renderer::get().beginFrame();
                Renderer::DX12::DX12Renderer::get().endFrame();
            #endif
        }
    }

    void Engine::shutdown() {
        #ifdef _WIN32
            Renderer::DX12::DX12Renderer::get().waitIdle();
            Renderer::DX12::DX12Renderer::get().shutdown();
            Platform::Win32_Window::get().shutdown();
        #endif
    }
}