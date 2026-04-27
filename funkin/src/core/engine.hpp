#pragma once
#include <string>
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
		RendererBackend renderer = RendererBackend::Vulkan;
	};

	class Engine {
	public:
		static Engine& get();

		void init(const EngineConfig& cfg);
		void run();
		void shutdown();
		void quit() { m_running = false; }

		const EngineConfig& config()    const { return m_cfg; }
		bool                isRunning() const { return m_running; }

	private:
		Engine() = default;

		EngineConfig      			m_cfg;
		bool              			m_running  = false;
		Renderer::IRenderer*        m_renderer = nullptr;
	};
}