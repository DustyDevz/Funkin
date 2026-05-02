// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "input.hpp"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <timeapi.h>

#ifdef _WIN32
    #include <platform/input/input_win32.hpp>
    #include <platform/window/window_win32.hpp>
    
    static XINPUT_STATE s_xiState[4]{};
    static bool         s_xiConnected[4]{};

    #pragma comment(lib, "winmm.lib")
#elif __linux

#endif

namespace Funkin::Input {
    Input& Input::get() {
        static Input s;
        return s;
    }

    void Input::init() {
        for (auto& dz : m_deadZones) {
            dz.inner = .15f;
            dz.outer = .95f;
        }

        m_startTime = Platform::Input::nanoTime();

        #ifdef _WIN32
            Platform::Input::registerRawInput(Funkin::Platform::Window_Win32::get().hwnd());

            m_threadRunning = true;
            m_inputThread = std::thread([this]() {
                timeBeginPeriod(1); 
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

                while (m_threadRunning) {
                    Platform::Input::pollXInput(m_ring, m_startTime, s_xiState, s_xiConnected);
                    std::this_thread::yield(); 
                }
                timeEndPeriod(1);
            });
        #elif __linux

        #endif
    };

    float Input::applyDeadZone(float value, const DeadZone& dz) const {
        float abs = value < 0 ? -value : value;
        float sign = value < 0 ? -1.0f : 1.0f;

        if (abs < dz.inner) return 0.0f;
        if (abs > dz.outer) return sign;

        float normalized = (abs - dz.inner) / (dz.outer - dz.inner);
        return sign * normalized;
    }

    void Input::setDeadZone(ControllerAxis axis, DeadZone dz) {
        m_deadZones[(size_t)axis] = dz;
    }

    DeadZone Input::deadZone(ControllerAxis axis) const {
        return m_deadZones[(size_t)axis];
    }

    uint64_t Input::getNow() const {
        return Platform::Input::nanoTime() - m_startTime;
    }

    uint64_t Input::getLastTimestamp(const std::string& action) const {
        auto it = m_bindings.find(action);
        if (it == m_bindings.end()) return 0;
        
        KeyCode targetKey = it->second.key;
        uint64_t latest = 0;

        for (const auto& e : m_state.frameEvents) {
            if (e.type == InputEventType::KeyDown && e.key.key == targetKey) {
                latest = e.time;
            }
        }
        return latest;
    }

    float Input::axis(uint8_t controller, ControllerAxis a) const {
        if (controller >= 4) return 0.0f;
        return m_state.controllers[controller].axes[(size_t)a].processed;
    }

    void Input::bind(const std::string& action, KeyCode key) {
        auto& b    = m_bindings[action];
        b.action   = action;
        b.key      = key;
        b.useKey   = true;
    }

    void Input::bind(const std::string& action, ControllerButton btn) {
        auto& b      = m_bindings[action];
        b.action     = action;
        b.ctrlBtn    = btn;
        b.useCtrl    = true;
    }

    void Input::unbind(const std::string& action) {
        m_bindings.erase(action);
    }

    void Input::shutdown() {
        m_threadRunning = false;
        if (m_inputThread.joinable())
            m_inputThread.join();
    }

    void Input::update() {
        m_state.keysJustDown.fill(false);
        m_state.keysJustUp.fill(false);
        m_state.mouseJustDown.fill(false);
        m_state.mouseJustUp.fill(false);
        m_state.mouseDX  = 0;
        m_state.mouseDY  = 0;
        m_state.scrollX  = 0;
        m_state.scrollY  = 0;
        m_state.frameEvents.clear();

        for (auto& ctrl : m_state.controllers) {
            ctrl.justDown.fill(false);
            ctrl.justUp.fill(false);
        }

        InputEvent e{};
        while (m_ring.pop(e)) {
            m_state.frameEvents.push_back(e);

            switch (e.type) {
                case InputEventType::KeyDown:
                    if (!m_state.keys[(size_t)e.key.key]) {
                        m_state.keysJustDown[(size_t)e.key.key] = true;
                    }
                    m_state.keys[(size_t)e.key.key] = true;
                    break;

                case InputEventType::KeyUp:
                    m_state.keys[(size_t)e.key.key]       = false;
                    m_state.keysJustUp[(size_t)e.key.key] = true;
                    break;

                case InputEventType::MouseMove:
                    m_state.mouseDX += e.mouseMove.x;
                    m_state.mouseDY += e.mouseMove.y;
                    m_state.mouseX  += e.mouseMove.x;
                    m_state.mouseY  += e.mouseMove.y;
                    break;

                case InputEventType::MouseDown:
                    m_state.mouseButtons[(size_t)e.mouseButton.btn]  = true;
                    m_state.mouseJustDown[(size_t)e.mouseButton.btn] = true;
                    break;

                case InputEventType::MouseUp:
                    m_state.mouseButtons[(size_t)e.mouseButton.btn] = false;
                    m_state.mouseJustUp[(size_t)e.mouseButton.btn]  = true;
                    break;

                case InputEventType::MouseScroll:
                    m_state.scrollX += e.mouseScroll.dx;
                    m_state.scrollY += e.mouseScroll.dy;
                    break;

                case InputEventType::ControllerButtonDown: {
                    auto& ctrl = m_state.controllers[e.ctrlBtn.id];
                    ctrl.buttons[(size_t)e.ctrlBtn.btn] = true;
                    ctrl.justDown[(size_t)e.ctrlBtn.btn] = true;
                    break;
                }

                case InputEventType::ControllerButtonUp: {
                    auto& ctrl = m_state.controllers[e.ctrlBtn.id];
                    ctrl.buttons[(size_t)e.ctrlBtn.btn] = false;
                    ctrl.justUp[(size_t)e.ctrlBtn.btn]  = true;
                    break;
                }

                case InputEventType::ControllerAxis: {
                    auto& ctrl  = m_state.controllers[e.ctrlAxis.id];
                    auto& axSt  = ctrl.axes[(size_t)e.ctrlAxis.axis];
                    axSt.raw       = e.ctrlAxis.value;
                    axSt.processed = applyDeadZone(
                        e.ctrlAxis.value,
                        m_deadZones[(size_t)e.ctrlAxis.axis]
                    );
                    break;
                }

                case InputEventType::ControllerConnected:
                    m_state.controllers[e.ctrlConnect.id].connected = true;
                    break;

                case InputEventType::ControllerDisconnected:
                    m_state.controllers[e.ctrlConnect.id].connected = false;
                    break;

                default: break;
            }
        }
    }

    bool Input::isDown(const std::string& action) const {
        auto it = m_bindings.find(action);
        if (it == m_bindings.end()) return false;
        auto& b = it->second;
        if (b.useKey  && m_state.keys[(size_t)b.key]) return true;
        if (b.useCtrl && m_state.controllers[0].buttons[(size_t)b.ctrlBtn]) return true;
        return false;
    }

    bool Input::justDown(const std::string& action) const {
        auto it = m_bindings.find(action);
        if (it == m_bindings.end()) return false;
        auto& b = it->second;
        if (b.useKey  && m_state.keysJustDown[(size_t)b.key]) return true;
        if (b.useCtrl && m_state.controllers[0].justDown[(size_t)b.ctrlBtn]) return true;
        return false;
    }

    bool Input::justUp(const std::string& action) const {
        auto it = m_bindings.find(action);
        if (it == m_bindings.end()) return false;
        auto& b = it->second;
        if (b.useKey  && m_state.keysJustUp[(size_t)b.key]) return true;
        if (b.useCtrl && m_state.controllers[0].justUp[(size_t)b.ctrlBtn]) return true;
        return false;
    }

    void Input::saveBindings(const std::string& path) const {
        std::ofstream f(path);
        if (!f.is_open()) return;

        for (auto& [name, b] : m_bindings) {
            if (b.useKey)
                f << name << "=key:" << (uint32_t)b.key << "\n";
            if (b.useCtrl)
                f << name << "=ctrl:" << (uint32_t)b.ctrlBtn << "\n";
        }
    }

    void Input::loadBindings(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return;

        std::string line;
        while (std::getline(f, line)) {
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string action = line.substr(0, eq);
            std::string value  = line.substr(eq + 1);

            if (value.starts_with("key:"))
                bind(action, (KeyCode)std::stoul(value.substr(4)));
            else if (value.starts_with("ctrl:"))
                bind(action, (ControllerButton)std::stoul(value.substr(5)));
        }
    }

    static const char* s_keyNames[(size_t)KeyCode::COUNT] = {
        "Unknown",
        "A","B","C","D","E","F","G","H","I","J","K","L","M",
        "N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
        "0","1","2","3","4","5","6","7","8","9",
        "Numpad0","Numpad1","Numpad2","Numpad3","Numpad4",
        "Numpad5","Numpad6","Numpad7","Numpad8","Numpad9",
        "NumpadAdd","NumpadSub","NumpadMul","NumpadDiv","NumpadEnter",
        "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
        "LShift","RShift","LCtrl","RCtrl","LAlt","RAlt","LSuper","RSuper",
        "Up","Down","Left","Right",
        "Home","End","PageUp","PageDown","Insert","Delete",
        "Enter","Escape","Space","Tab","Backspace","CapsLock",
        "PrintScreen","ScrollLock","Pause",
        "Comma","Period","Slash","Backslash",
        "Semicolon","Apostrophe","Grave","LBracket","RBracket","Minus","Equal"
    };

    const char* keyCodeName(KeyCode k) {
        size_t i = (size_t)k;
        if (i >= (size_t)KeyCode::COUNT) return "Unknown";
        return s_keyNames[i];
    }

    KeyCode keyCodeFromName(const char* name) {
        for (size_t i = 0; i < (size_t)KeyCode::COUNT; ++i)
            if (strcmp(s_keyNames[i], name) == 0)
                return (KeyCode)i;
        return KeyCode::Unknown;
    }
}