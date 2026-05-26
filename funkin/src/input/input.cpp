// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "input.hpp"
#include <fstream>
#include <cstring>
#include <SDL3/SDL_scancode.h>

namespace Funkin::Input {
    static KeyCode mapSDLScancode(SDL_Scancode code) {
        if (code >= SDL_SCANCODE_A && code <= SDL_SCANCODE_Z) {
            return static_cast<KeyCode>(1 + (code - SDL_SCANCODE_A));
        }
        if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9) {
            return static_cast<KeyCode>(27 + (code - SDL_SCANCODE_1));
        }
        if (code == SDL_SCANCODE_0) return KeyCode::Num0;

        switch (code) {
            case SDL_SCANCODE_KP_0: return KeyCode::Numpad0;
            case SDL_SCANCODE_KP_1: return KeyCode::Numpad1;
            case SDL_SCANCODE_KP_2: return KeyCode::Numpad2;
            case SDL_SCANCODE_KP_3: return KeyCode::Numpad3;
            case SDL_SCANCODE_KP_4: return KeyCode::Numpad4;
            case SDL_SCANCODE_KP_5: return KeyCode::Numpad5;
            case SDL_SCANCODE_KP_6: return KeyCode::Numpad6;
            case SDL_SCANCODE_KP_7: return KeyCode::Numpad7;
            case SDL_SCANCODE_KP_8: return KeyCode::Numpad8;
            case SDL_SCANCODE_KP_9: return KeyCode::Numpad9;
            case SDL_SCANCODE_KP_PLUS: return KeyCode::NumpadAdd;
            case SDL_SCANCODE_KP_MINUS: return KeyCode::NumpadSub;
            case SDL_SCANCODE_KP_MULTIPLY: return KeyCode::NumpadMul;
            case SDL_SCANCODE_KP_DIVIDE: return KeyCode::NumpadDiv;
            case SDL_SCANCODE_KP_ENTER: return KeyCode::NumpadEnter;
            case SDL_SCANCODE_F1: return KeyCode::F1;
            case SDL_SCANCODE_F2: return KeyCode::F2;
            case SDL_SCANCODE_F3: return KeyCode::F3;
            case SDL_SCANCODE_F4: return KeyCode::F4;
            case SDL_SCANCODE_F5: return KeyCode::F5;
            case SDL_SCANCODE_F6: return KeyCode::F6;
            case SDL_SCANCODE_F7: return KeyCode::F7;
            case SDL_SCANCODE_F8: return KeyCode::F8;
            case SDL_SCANCODE_F9: return KeyCode::F9;
            case SDL_SCANCODE_F10: return KeyCode::F10;
            case SDL_SCANCODE_F11: return KeyCode::F11;
            case SDL_SCANCODE_F12: return KeyCode::F12;
            case SDL_SCANCODE_LSHIFT: return KeyCode::LShift;
            case SDL_SCANCODE_RSHIFT: return KeyCode::RShift;
            case SDL_SCANCODE_LCTRL: return KeyCode::LCtrl;
            case SDL_SCANCODE_RCTRL: return KeyCode::RCtrl;
            case SDL_SCANCODE_LALT: return KeyCode::LAlt;
            case SDL_SCANCODE_RALT: return KeyCode::RAlt;
            case SDL_SCANCODE_LGUI: return KeyCode::LSuper;
            case SDL_SCANCODE_RGUI: return KeyCode::RSuper;
            case SDL_SCANCODE_UP: return KeyCode::Up;
            case SDL_SCANCODE_DOWN: return KeyCode::Down;
            case SDL_SCANCODE_LEFT: return KeyCode::Left;
            case SDL_SCANCODE_RIGHT: return KeyCode::Right;
            case SDL_SCANCODE_HOME: return KeyCode::Home;
            case SDL_SCANCODE_END: return KeyCode::End;
            case SDL_SCANCODE_PAGEUP: return KeyCode::PageUp;
            case SDL_SCANCODE_PAGEDOWN: return KeyCode::PageDown;
            case SDL_SCANCODE_INSERT: return KeyCode::Insert;
            case SDL_SCANCODE_DELETE: return KeyCode::Delete;
            case SDL_SCANCODE_RETURN: return KeyCode::Enter;
            case SDL_SCANCODE_ESCAPE: return KeyCode::Escape;
            case SDL_SCANCODE_SPACE: return KeyCode::Space;
            case SDL_SCANCODE_TAB: return KeyCode::Tab;
            case SDL_SCANCODE_BACKSPACE: return KeyCode::Backspace;
            case SDL_SCANCODE_CAPSLOCK: return KeyCode::CapsLock;
            case SDL_SCANCODE_PRINTSCREEN: return KeyCode::PrintScreen;
            case SDL_SCANCODE_SCROLLLOCK: return KeyCode::ScrollLock;
            case SDL_SCANCODE_PAUSE: return KeyCode::Pause;
            case SDL_SCANCODE_COMMA: return KeyCode::Comma;
            case SDL_SCANCODE_PERIOD: return KeyCode::Period;
            case SDL_SCANCODE_SLASH: return KeyCode::Slash;
            case SDL_SCANCODE_BACKSLASH: return KeyCode::Backslash;
            case SDL_SCANCODE_SEMICOLON: return KeyCode::Semicolon;
            case SDL_SCANCODE_APOSTROPHE: return KeyCode::Apostrophe;
            case SDL_SCANCODE_GRAVE: return KeyCode::Grave;
            case SDL_SCANCODE_LEFTBRACKET: return KeyCode::LBracket;
            case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::RBracket;
            case SDL_SCANCODE_MINUS: return KeyCode::Minus;
            case SDL_SCANCODE_EQUALS: return KeyCode::Equal;
            default: return KeyCode::Unknown;
        }
    }

    static ControllerButton mapSDLGamepadButton(SDL_GamepadButton btn) {
        switch (btn) {
            case SDL_GAMEPAD_BUTTON_SOUTH: return ControllerButton::A;
            case SDL_GAMEPAD_BUTTON_EAST: return ControllerButton::B;
            case SDL_GAMEPAD_BUTTON_WEST: return ControllerButton::X;
            case SDL_GAMEPAD_BUTTON_NORTH: return ControllerButton::Y;
            case SDL_GAMEPAD_BUTTON_BACK: return ControllerButton::Back;
            case SDL_GAMEPAD_BUTTON_GUIDE: return ControllerButton::Guide;
            case SDL_GAMEPAD_BUTTON_START: return ControllerButton::Start;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK: return ControllerButton::LeftStick;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return ControllerButton::RightStick;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return ControllerButton::LeftShoulder;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return ControllerButton::RightShoulder;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: return ControllerButton::DpadUp;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return ControllerButton::DpadDown;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return ControllerButton::DpadLeft;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return ControllerButton::DpadRight;
            default: return ControllerButton::COUNT;
        }
    }

    static ControllerAxis mapSDLGamepadAxis(SDL_GamepadAxis axis) {
        switch (axis) {
            case SDL_GAMEPAD_AXIS_LEFTX: return ControllerAxis::LeftX;
            case SDL_GAMEPAD_AXIS_LEFTY: return ControllerAxis::LeftY;
            case SDL_GAMEPAD_AXIS_RIGHTX: return ControllerAxis::RightX;
            case SDL_GAMEPAD_AXIS_RIGHTY: return ControllerAxis::RightY;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return ControllerAxis::TriggerLeft;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return ControllerAxis::TriggerRight;
            default: return ControllerAxis::COUNT;
        }
    }

    Input& Input::get() {
        static Input s;
        return s;
    }

    void Input::init() {
        for (auto& dz : m_deadZones) {
            dz.inner = .15f;
            dz.outer = .95f;
        }
        m_startTime = SDL_GetTicksNS();
        m_threadRunning = false;
    }

    void Input::shutdown() {
    }

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
        return SDL_GetTicksNS() - m_startTime;
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

    void Input::syncMousePosition() {
        if (!m_windowHandle) return;
        float x = 0.0f;
        float y = 0.0f;
        SDL_GetMouseState(&x, &y);
        m_state.mouseX = x;
        m_state.mouseY = y;
    }

    void Input::handleSDLEvent(const SDL_Event& event) {
        InputEvent e{};
        e.time = getNow();

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                e.type = (event.type == SDL_EVENT_KEY_DOWN) ? InputEventType::KeyDown : InputEventType::KeyUp;
                e.key.key = mapSDLScancode(event.key.scancode);
                if (e.key.key != KeyCode::Unknown) {
                    m_ring.push(e);
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                e.type = InputEventType::MouseMove;
                e.mouseMove.x = event.motion.xrel;
                e.mouseMove.y = event.motion.yrel;
                m_ring.push(e);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                e.type = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? InputEventType::MouseDown : InputEventType::MouseUp;
                if (event.button.button == SDL_BUTTON_LEFT)   e.mouseButton.btn = MouseButton::Left;
                if (event.button.button == SDL_BUTTON_RIGHT)  e.mouseButton.btn = MouseButton::Right;
                if (event.button.button == SDL_BUTTON_MIDDLE) e.mouseButton.btn = MouseButton::Middle;
                m_ring.push(e);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                e.type = InputEventType::MouseScroll;
                e.mouseScroll.dx = event.wheel.x;
                e.mouseScroll.dy = event.wheel.y;
                m_ring.push(e);
                break;
            }
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                e.type = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) ? InputEventType::ControllerButtonDown : InputEventType::ControllerButtonUp;
                e.ctrlBtn.id = static_cast<uint8_t>(event.gbutton.which);
                e.ctrlBtn.btn = mapSDLGamepadButton(static_cast<SDL_GamepadButton>(event.gbutton.button));
                
                if (e.ctrlBtn.btn != ControllerButton::COUNT) {
                    m_ring.push(e);
                }
                break;
            }
            case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                e.type = InputEventType::ControllerAxis;
                e.ctrlAxis.id = static_cast<uint8_t>(event.gaxis.which);
                e.ctrlAxis.axis = mapSDLGamepadAxis(static_cast<SDL_GamepadAxis>(event.gaxis.axis));
                
                if (e.ctrlAxis.axis != ControllerAxis::COUNT) {
                    e.ctrlAxis.value = static_cast<float>(event.gaxis.value) / 32767.0f;
                    m_ring.push(e);
                }
                break;
            }
            case SDL_EVENT_GAMEPAD_ADDED: {
                e.type = InputEventType::ControllerConnected;
                e.ctrlConnect.id = static_cast<uint8_t>(event.gdevice.which);
                SDL_OpenGamepad(event.gdevice.which);
                m_ring.push(e);
                break;
            }
            case SDL_EVENT_GAMEPAD_REMOVED: {
                e.type = InputEventType::ControllerDisconnected;
                e.ctrlConnect.id = static_cast<uint8_t>(event.gdevice.which);
                m_ring.push(e);
                break;
            }
            default:
                break;
        }
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
        //std::ofstream f(path);
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