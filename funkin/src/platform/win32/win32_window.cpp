// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "win32_window.hpp"
#include <bgfx/bgfx.h>
#include "input/input.hpp"
#include "platform/win32/win32_input.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

ViewportWidget::ViewportWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
}

bool ViewportWidget::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_ERASEBKGND) {
            *result = 1;
            return true;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

bool RawInputFilter::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_ENTERSIZEMOVE) windowMoving = true;
        if (msg->message == WM_EXITSIZEMOVE)  windowMoving = false;

        if (viewportWidget && viewportWidget->internalWinId()) {
            HWND vpHwnd = (HWND)viewportWidget->winId();
            if (msg->hwnd == vpHwnd) {
                if (msg->message == WM_ERASEBKGND) {
                    *result = 1;
                    return true;
                }

                if (msg->message == WM_SIZE && bgfx::getRendererType() != bgfx::RendererType::Noop) {
                    int w = LOWORD(msg->lParam);
                    int h = HIWORD(msg->lParam);
                    if (w > 0 && h > 0) {
                        bgfx::reset((uint32_t)w, (uint32_t)h, BGFX_RESET_VSYNC);
                        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
                        bgfx::setViewRect(0, 0, 0, (uint16_t)w, (uint16_t)h);
                        if (onRenderFrame) onRenderFrame();
                    }
                }

                if (msg->message == WM_PAINT && windowMoving && bgfx::getRendererType() != bgfx::RendererType::Noop) {
                    if (onRenderFrame) onRenderFrame();
                }
            }
        }

        if (msg->message == WM_INPUT) {
            Funkin::Platform::Input::handleRawInput(
                (HRAWINPUT)msg->lParam,
                Funkin::Input::Input::get().ring(),
                Funkin::Input::Input::get().startTime(),
                Funkin::Platform::Input::nanoTime()
            );
            return false;
        }

        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
        ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam);
    }
    return false;
}