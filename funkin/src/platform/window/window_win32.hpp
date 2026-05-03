// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#ifdef _WIN32
#include "window.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

namespace Funkin::Platform {
    class Window_Win32 : public Window {
    public:
        static Window_Win32& get();

        bool init(const std::string& title, int w, int h) override;
        bool pump()     override;
        bool shutdown() override;

        HWND      hwnd()      const { return m_hwnd; }
        HINSTANCE hinstance() const { return m_hinstance; }

    private:
        Window_Win32() = default;
        static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);

        HWND      m_hwnd      = nullptr;
        HINSTANCE m_hinstance = nullptr;
    };
}

#endif