// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "engine.hpp"
#include "renderer/renderer.hpp"
#include <renderer/vk/vk_renderer.hpp>

#ifdef _WIN32
    #include <platform/window/window_win32.hpp>
    #include <renderer/dx12/dx12_renderer.hpp>
    using PlatformWindow = Funkin::Platform::Window_Win32;
    #elif __linux__
    using PlatformWindow = Funkin::Platform::X11_Window;
#endif

namespace Funkin::Core {
    Engine& Engine::get() {
        static Engine s;
        return s;
    }

    bool Engine::init(const EngineConfig& cfg) {
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
        return true;
    }

    void Engine::beginFrame() {
        PlatformWindow::get().pump() ? void() : quit();
        m_renderer->beginFrame();
    }

    void Engine::endFrame() {
        m_renderer->endFrame();
    }

    void Engine::run() {
        while (m_running) {
            beginFrame();
            endFrame();
        }
        m_renderer->waitIdle();
    }

    void Engine::shutdown() {
        m_renderer->shutdown();
        PlatformWindow::get().shutdown();
    }
}
