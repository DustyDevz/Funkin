#pragma once
#ifdef __linux__

#include "window.hpp"
#include <X11/Xlib.h>

namespace Funkin::Platform {

    class X11Window : public Window {
    public:
        static X11Window& get();

        void init(const std::string& title, int w, int h) override;
        bool pump() override;
        void shutdown() override;

        Display* display() const { return m_display; }
        ::Window window()  const { return m_window; }

    private:
        X11Window() = default;

        Display* m_display = nullptr;
        ::Window m_window = 0;
        Atom     m_deleteMsg;
    };

}

#endif