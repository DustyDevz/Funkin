// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <renderer/gal/idal.hpp>

namespace Funkin::Core {
    enum class RendererBackend {
        Vulkan,
        #ifdef _WIN32
        DX12,
        #endif
    };

    struct EngineConfig {
        std::string     title    = "Funkin";
        int             width    = 1280;
        int             height   = 720;
        bool            vsync    = false;
        #ifdef _WIN32
            RendererBackend renderer = RendererBackend::DX12;
        #else
            RendererBackend renderer = RendererBackend::Vulkan;
        #endif
    };

    using FrameCallback = std::function<void()>;
    using ResizeCallback = std::function<void(uint32_t, uint32_t)>;

    class Engine {
    public:
        static Engine& get();

        bool init(const EngineConfig& cfg);
        void run();
        void shutdown();
        void quit() { m_running = false; }

        bool processEvents();
        void tickFrame();
        void forceFrame() { tickFrame(); }
        void resize(int w, int h);

        void setFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }
        void setResizeCallback(ResizeCallback cb) { m_resizeCallback = std::move(cb); }
        void setPreResizeCallback(std::function<void()> cb) { m_preResizeCallback = std::move(cb); }

        Renderer::GAL::IDAL* gal()           const { return m_renderer; }
        const EngineConfig&  config()       const { return m_cfg; }
        bool                 isRunning()    const { return m_running; }

        bool m_resizing = false;

    private:
        Engine() = default;

        EngineConfig                           m_cfg;
        bool                                   m_running      = false;
        std::unique_ptr<Renderer::GAL::IDAL>   m_galOwner;
        Renderer::GAL::IDAL*                   m_renderer     = nullptr;
        FrameCallback                          m_frameCallback;
        ResizeCallback                         m_resizeCallback;
        std::function<void()>                  m_preResizeCallback;
    };
}