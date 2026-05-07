// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "engine.hpp"
#include <renderer/gal/vk/vk_gal.hpp>
#include <input/input.hpp>

#ifdef _WIN32
    #include <platform/window/window_win32.hpp>
    #include <renderer/gal/dx12/dx12_gal.hpp>
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
        Input::Input::get().init();

        #ifdef _WIN32
            if (cfg.renderer == RendererBackend::DX12)
                m_galOwner = std::make_unique<Renderer::GAL::DX12Gal>();
        else
            m_galOwner = std::make_unique<Renderer::GAL::VKGal>();
        #elif __linux__
            m_galOwner = std::make_unique<Renderer::GAL::VKGal>();
        #endif

        m_renderer = m_galOwner.get();

        Renderer::GAL::GALDesc galDesc{};
        galDesc.width  = static_cast<uint32_t>(cfg.width);
        galDesc.height = static_cast<uint32_t>(cfg.height);
        galDesc.vsync  = cfg.vsync;

        #ifdef _WIN32
            galDesc.windowHandle = static_cast<void*>(PlatformWindow::get().hwnd());
        #endif

        m_renderer->init(galDesc);
        return true;
    }

    bool Engine::processEvents() {
        if (!PlatformWindow::get().pump()) {
            quit();
            return false;
        }
        Input::Input::get().update();
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
        if (w == m_cfg.width && h == m_cfg.height) return;

        m_cfg.width  = w;
        m_cfg.height = h;

        if (m_preResizeCallback) 
            m_preResizeCallback();

        m_renderer->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));

        if (m_resizeCallback)
            m_resizeCallback(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        
        tickFrame();
    }

    void Engine::shutdown() {
        if (m_renderer) {
            m_renderer->shutdown();
            m_renderer = nullptr;
        }
        m_galOwner.reset();
        Input::Input::get().shutdown();
        PlatformWindow::get().shutdown();
    }
}
