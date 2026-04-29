// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <string>
#include <functional>
#include <renderer/renderer.hpp>

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
        RendererBackend renderer = RendererBackend::DX12;
    };

    using FrameCallback = std::function<void()>;

    class Engine {
    public:
        static Engine& get();

        bool init(const EngineConfig& cfg);
        void run();
        void shutdown();
        void quit() { m_running = false; }

        bool processEvents();
        void tickFrame();

        void resize(int w, int h);
        void setFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

        const EngineConfig& config()    const { return m_cfg; }
        bool                isRunning() const { return m_running; }

    private:
        Engine() = default;

        EngineConfig         m_cfg;
        bool                 m_running      = false;
        Renderer::IRenderer* m_renderer     = nullptr;
        FrameCallback        m_frameCallback;
    };

}