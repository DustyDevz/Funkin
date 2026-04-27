// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#ifdef _WIN32

#include "window_win32.hpp"
#include "core/engine.hpp"
#include <stdexcept>

namespace Funkin::Platform {
    Window_Win32& Window_Win32::get() {
        static Window_Win32 s;
        return s;
    }

    bool Window_Win32::init(const std::string& title, int w, int h) {
        m_hinstance = GetModuleHandle(nullptr);

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = wndProc;
        wc.hInstance = m_hinstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.lpszClassName = L"FunkinWindow";

        if (!RegisterClassExW(&wc))
            throw std::runtime_error("Failed to register window class");

        RECT r = { 0, 0, w, h };
        AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

        std::wstring wt(title.begin(), title.end());
        m_hwnd = CreateWindowExW(
            0, L"FunkinWindow", wt.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            r.right - r.left, r.bottom - r.top,
            nullptr, nullptr, m_hinstance, nullptr
        );

        if (!m_hwnd)
            throw std::runtime_error("Failed to create window");

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);

        return true;
    }

    bool Window_Win32::pump() {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

    bool Window_Win32::shutdown() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        UnregisterClassW(L"FunkinWindow", m_hinstance);
        return true;
    }

    LRESULT CALLBACK Window_Win32::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            Core::Engine::get().quit();
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

#endif