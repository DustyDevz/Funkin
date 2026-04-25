#include "engine.hpp"
#include <platform/win32_window.hpp>
#include <renderer/dx12/renderer_dx12.hpp>

namespace Funkin::Core {
	Engine& Engine::get() {
		static Engine s;
		return s;
	}

	void Engine::init(const EngineConfig& cfg) {
		m_cfg	  = cfg;
		m_running = true;

		Platform::Win32_Window::get().init(cfg.title, cfg.width, cfg.height);
		Renderer::DX12::DX12Renderer::get().init(cfg.width, cfg.height, cfg.vsync);
	}

	void Engine::run() {
		while (m_running) {
			if (!Platform::Win32_Window::get().pump()) {
				quit();
				break;
			}
			Renderer::DX12::DX12Renderer::get().beginFrame();
			Renderer::DX12::DX12Renderer::get().endFrame();
		}
		Renderer::DX12::DX12Renderer::get().waitIdle();
	}

	void Engine::shutdown() {
		Renderer::DX12::DX12Renderer::get().shutdown();
		Platform::Win32_Window::get().shutdown();
	}
}