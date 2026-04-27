// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "engine.hpp"
#include "renderer/renderer.hpp"

#ifdef _WIN32
    #include <platform/window/window_win32.hpp>
    #include <renderer/dx12/dx12_renderer.hpp>
    #include <renderer/vk/vk_renderer.hpp>
    using PlatformWindow = Funkin::Platform::Window_Win32;
#endif

namespace Funkin::Core {
    Engine& Engine::get() {
        static Engine s;
        return s;
    }

    void Engine::init(const EngineConfig& cfg) {
        m_cfg     = cfg;
        m_running = true;

        PlatformWindow::get().init(cfg.title, cfg.width, cfg.height);

    #ifdef _WIN32
        if (cfg.renderer == RendererBackend::DX12)
            m_renderer = &Renderer::DX12::DX12Renderer::get();
        else
            m_renderer = &Renderer::VK::VK_Renderer::get();
    #elif __linux__
        m_renderer = &Renderer::VK::VK_Renderer::get();
    #endif

        m_renderer->init(cfg.width, cfg.height, cfg.vsync);
    }

    void Engine::run() {
        while (m_running) {
            if (!PlatformWindow::get().pump()) {
                quit();
                break;
            }
            m_renderer->beginFrame();
            m_renderer->endFrame();
        }
        m_renderer->waitIdle();
    }

    void Engine::shutdown() {
        m_renderer->shutdown();
        PlatformWindow::get().shutdown();
    }
}