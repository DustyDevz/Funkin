#pragma once

namespace Funkin::Renderer {
	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		virtual void init(int width, int height, bool vsync) = 0;
		virtual void beginFrame() = 0;
		virtual void endFrame()   = 0;
		virtual void waitIdle()   = 0;
		virtual void shutdown()   = 0;
	};
} 