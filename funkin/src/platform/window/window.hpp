#pragma once
#include <string>
#include <memory>

namespace Funkin::Platform {
	class Window {
	public:
		virtual ~Window() = default;

		virtual bool init(const std::string& title, int w, int h) = 0;
		virtual bool pump()		= 0;
		virtual bool shutdown() = 0;

		static std::unique_ptr<Window> create();
	};
}