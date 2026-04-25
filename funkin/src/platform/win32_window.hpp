#pragma once
#ifdef _WIN32

#include "window.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Funkin::Platform {

    class Win32_Window : public Window {
    public:
        static Win32_Window& get();

        bool init(const std::string& title, int w, int h) override;
        bool pump()     override;
        bool shutdown() override;

        HWND      hwnd()      const { return m_hwnd; }
        HINSTANCE hinstance() const { return m_hinstance; }

    private:
        Win32_Window() = default;
        static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);

        HWND      m_hwnd = nullptr;
        HINSTANCE m_hinstance = nullptr;
    };

}

#endif