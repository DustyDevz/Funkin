#pragma once
#include <string>

namespace Funkin::Core {
	struct EngineConfig {
		std::string title = "test";
		int         width = 1280;
		int         height = 720;
		bool        vsync = false;
	};

	class Engine {
	public:
		static Engine& get();

		void init(const EngineConfig& cfg);
		void run();
		void shutdown();
		void quit() { m_running; }

		const EngineConfig& config() const { return m_cfg; }
		bool  isRunning()            const { return m_running; }

	private:
		Engine() = default;

		EngineConfig m_cfg;
		bool		 m_running = false;
	};
};