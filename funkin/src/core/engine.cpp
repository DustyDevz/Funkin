// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "engine.hpp"
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

    bool Engine::processEvents() {
        if (!PlatformWindow::get().pump()) {
            quit();
            return false;
        }
        return m_running;
    }

    void Engine::tickFrame() {
        if (!m_running || !m_renderer) return;
        m_renderer->beginFrame();
        if (m_frameCallback) m_frameCallback();
        m_renderer->endFrame();
    }

    void Engine::run() {
        while (m_running) {
            processEvents();
            tickFrame();
        }
        m_renderer->waitIdle();
    }

    void Engine::resize(int w, int h) {
        if (w <= 0 || h <= 0 || !m_renderer) return;
        m_cfg.width  = w;
        m_cfg.height = h;
        m_renderer->resize(w, h);
    }

    void Engine::shutdown() {
        m_renderer->shutdown();
        PlatformWindow::get().shutdown();
    }
}