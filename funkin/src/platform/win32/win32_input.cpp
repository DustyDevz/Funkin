// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "win32_input.hpp"
#pragma comment(lib, "xinput.lib")

#ifdef _WIN32
    namespace Funkin::Platform::Input {
        uint64_t nanoTime() {
            static LARGE_INTEGER freq = [] {
                LARGE_INTEGER f; QueryPerformanceFrequency(&f); return f;
            }();
            LARGE_INTEGER count;
            QueryPerformanceCounter(&count);
            return (uint64_t)(count.QuadPart * 1'000'000'000LL / freq.QuadPart);
        }

        void registerRawInput(HWND hwnd) {
            RAWINPUTDEVICE devices[2];

            devices[0].usUsagePage = 0x01;
            devices[0].usUsage     = 0x06;
            devices[0].dwFlags     = RIDEV_NOLEGACY; 
            devices[0].hwndTarget  = hwnd;

            devices[1].usUsagePage = 0x01;
            devices[1].usUsage     = 0x02;
            devices[1].dwFlags     = 0;
            devices[1].hwndTarget  = hwnd;

            RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
        }

        KeyCode vkToKeyCode(WPARAM vk, LPARAM flags) {
            bool extended = (flags >> 24) & 1;

            switch (vk) {
                case 'A': return KeyCode::A; case 'B': return KeyCode::B;
                case 'C': return KeyCode::C; case 'D': return KeyCode::D;
                case 'E': return KeyCode::E; case 'F': return KeyCode::F;
                case 'G': return KeyCode::G; case 'H': return KeyCode::H;
                case 'I': return KeyCode::I; case 'J': return KeyCode::J;
                case 'K': return KeyCode::K; case 'L': return KeyCode::L;
                case 'M': return KeyCode::M; case 'N': return KeyCode::N;
                case 'O': return KeyCode::O; case 'P': return KeyCode::P;
                case 'Q': return KeyCode::Q; case 'R': return KeyCode::R;
                case 'S': return KeyCode::S; case 'T': return KeyCode::T;
                case 'U': return KeyCode::U; case 'V': return KeyCode::V;
                case 'W': return KeyCode::W; case 'X': return KeyCode::X;
                case 'Y': return KeyCode::Y; case 'Z': return KeyCode::Z;

                case '0': return KeyCode::Num0; case '1': return KeyCode::Num1;
                case '2': return KeyCode::Num2; case '3': return KeyCode::Num3;
                case '4': return KeyCode::Num4; case '5': return KeyCode::Num5;
                case '6': return KeyCode::Num6; case '7': return KeyCode::Num7;
                case '8': return KeyCode::Num8; case '9': return KeyCode::Num9;

                case VK_NUMPAD0: return KeyCode::Numpad0;
                case VK_NUMPAD1: return KeyCode::Numpad1;
                case VK_NUMPAD2: return KeyCode::Numpad2;
                case VK_NUMPAD3: return KeyCode::Numpad3;
                case VK_NUMPAD4: return KeyCode::Numpad4;
                case VK_NUMPAD5: return KeyCode::Numpad5;
                case VK_NUMPAD6: return KeyCode::Numpad6;
                case VK_NUMPAD7: return KeyCode::Numpad7;
                case VK_NUMPAD8: return KeyCode::Numpad8;
                case VK_NUMPAD9: return KeyCode::Numpad9;
                case VK_ADD:     return KeyCode::NumpadAdd;
                case VK_SUBTRACT:return KeyCode::NumpadSub;
                case VK_MULTIPLY:return KeyCode::NumpadMul;
                case VK_DIVIDE:  return KeyCode::NumpadDiv;

                case VK_F1:  return KeyCode::F1;  case VK_F2:  return KeyCode::F2;
                case VK_F3:  return KeyCode::F3;  case VK_F4:  return KeyCode::F4;
                case VK_F5:  return KeyCode::F5;  case VK_F6:  return KeyCode::F6;
                case VK_F7:  return KeyCode::F7;  case VK_F8:  return KeyCode::F8;
                case VK_F9:  return KeyCode::F9;  case VK_F10: return KeyCode::F10;
                case VK_F11: return KeyCode::F11; case VK_F12: return KeyCode::F12;

                case VK_SHIFT:   return extended ? KeyCode::RShift : KeyCode::LShift;
                case VK_CONTROL: return extended ? KeyCode::RCtrl  : KeyCode::LCtrl;
                case VK_MENU:    return extended ? KeyCode::RAlt   : KeyCode::LAlt;
                case VK_LWIN:    return KeyCode::LSuper;
                case VK_RWIN:    return KeyCode::RSuper;

                case VK_UP:    return KeyCode::Up;    case VK_DOWN:  return KeyCode::Down;
                case VK_LEFT:  return KeyCode::Left;  case VK_RIGHT: return KeyCode::Right;
                case VK_HOME:  return KeyCode::Home;  case VK_END:   return KeyCode::End;
                case VK_PRIOR: return KeyCode::PageUp;
                case VK_NEXT:  return KeyCode::PageDown;
                case VK_INSERT:return KeyCode::Insert;
                case VK_DELETE:return KeyCode::Delete;

                case VK_RETURN: return extended ? KeyCode::NumpadEnter : KeyCode::Enter;
                case VK_ESCAPE: return KeyCode::Escape;
                case VK_SPACE:  return KeyCode::Space;
                case VK_TAB:    return KeyCode::Tab;
                case VK_BACK:   return KeyCode::Backspace;
                case VK_CAPITAL:return KeyCode::CapsLock;
                case VK_SNAPSHOT:return KeyCode::PrintScreen;
                case VK_SCROLL: return KeyCode::ScrollLock;
                case VK_PAUSE:  return KeyCode::Pause;

                case VK_OEM_COMMA:  return KeyCode::Comma;
                case VK_OEM_PERIOD: return KeyCode::Period;
                case VK_OEM_2:      return KeyCode::Slash;
                case VK_OEM_5:      return KeyCode::Backslash;
                case VK_OEM_1:      return KeyCode::Semicolon;
                case VK_OEM_7:      return KeyCode::Apostrophe;
                case VK_OEM_3:      return KeyCode::Grave;
                case VK_OEM_4:      return KeyCode::LBracket;
                case VK_OEM_6:      return KeyCode::RBracket;
                case VK_OEM_MINUS:  return KeyCode::Minus;
                case VK_OEM_PLUS:   return KeyCode::Equal;

                default: return KeyCode::Unknown;
            }
        }

        void handleRawInput(HRAWINPUT hRaw,
                            InputRingBuffer<4096>& ring,
                            uint64_t startTime,
                            uint64_t capturedNow) {
            UINT size = 0;
            GetRawInputData(hRaw, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

            static uint8_t buf[1024];
            if (size > sizeof(buf)) return;
            GetRawInputData(hRaw, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER));

            RAWINPUT* raw = (RAWINPUT*)buf;
            uint64_t  now = capturedNow - startTime;

            if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                auto& kb  = raw->data.keyboard;
                bool  up  = (kb.Flags & RI_KEY_BREAK) != 0;

                KeyCode key = vkToKeyCode(kb.VKey, kb.Flags);
                if (key == KeyCode::Unknown) return;

                InputEvent e{};
                e.type     = up ? InputEventType::KeyUp : InputEventType::KeyDown;
                e.time     = now;
                e.key.key    = key;
                e.key.repeat = false;
                ring.push(e);

            } else if (raw->header.dwType == RIM_TYPEMOUSE) {
                auto& m = raw->data.mouse;

                if (m.lLastX != 0 || m.lLastY != 0) {
                    InputEvent e{};
                    e.type         = InputEventType::MouseMove;
                    e.time         = now;
                    e.mouseMove.x  = (float)m.lLastX;
                    e.mouseMove.y  = (float)m.lLastY;
                    ring.push(e);
                }

                auto pushMouse = [&](MouseButton btn, bool down) {
                    InputEvent e{};
                    e.type           = down ? InputEventType::MouseDown : InputEventType::MouseUp;
                    e.time           = now;
                    e.mouseButton.btn = btn;
                    ring.push(e);
                };

                if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)   pushMouse(MouseButton::Left,   true);
                if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)     pushMouse(MouseButton::Left,   false);
                if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) pushMouse(MouseButton::Right,  true);
                if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)   pushMouse(MouseButton::Right,  false);
                if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)pushMouse(MouseButton::Middle, true);
                if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)  pushMouse(MouseButton::Middle, false);

                if (m.usButtonFlags & RI_MOUSE_WHEEL) {
                    InputEvent e{};
                    e.type          = InputEventType::MouseScroll;
                    e.time          = now;
                    e.mouseScroll.dy = (float)(short)m.usButtonData / WHEEL_DELTA;
                    ring.push(e);
                }
            }
        }

        static XINPUT_STATE s_lastXInput[4]{};
        static bool         s_xConnected[4]{};

        void pollXInput(InputRingBuffer<4096>& ring,
                        uint64_t startTime,
                        XINPUT_STATE lastState[4],
                        bool connected[4]) {
            uint64_t now = nanoTime() - startTime;

            for (DWORD i = 0; i < 4; ++i) {
                XINPUT_STATE state{};
                bool wasConnected = connected[i];
                bool isConnected  = XInputGetState(i, &state) == ERROR_SUCCESS;

                if (isConnected != wasConnected) {
                    connected[i] = isConnected;
                    InputEvent e{};
                    e.type           = isConnected
                                     ? InputEventType::ControllerConnected
                                     : InputEventType::ControllerDisconnected;
                    e.time           = now;
                    e.ctrlConnect.id = (uint8_t)i;
                    ring.push(e);
                    if (!isConnected) continue;
                }

                if (!isConnected) continue;

                auto& prev = lastState[i].Gamepad;
                auto& curr = state.Gamepad;

                auto checkBtn = [&](WORD mask, ControllerButton btn) {
                    bool wasDown = (prev.wButtons & mask) != 0;
                    bool isDown  = (curr.wButtons & mask) != 0;
                    if (wasDown == isDown) return;

                    InputEvent e{};
                    e.type       = isDown
                                 ? InputEventType::ControllerButtonDown
                                 : InputEventType::ControllerButtonUp;
                    e.time       = now;
                    e.ctrlBtn.id  = (uint8_t)i;
                    e.ctrlBtn.btn = btn;
                    ring.push(e);
                };

                checkBtn(XINPUT_GAMEPAD_A,              ControllerButton::A);
                checkBtn(XINPUT_GAMEPAD_B,              ControllerButton::B);
                checkBtn(XINPUT_GAMEPAD_X,              ControllerButton::X);
                checkBtn(XINPUT_GAMEPAD_Y,              ControllerButton::Y);
                checkBtn(XINPUT_GAMEPAD_LEFT_SHOULDER,  ControllerButton::LeftShoulder);
                checkBtn(XINPUT_GAMEPAD_RIGHT_SHOULDER, ControllerButton::RightShoulder);
                checkBtn(XINPUT_GAMEPAD_LEFT_THUMB,     ControllerButton::LeftStick);
                checkBtn(XINPUT_GAMEPAD_RIGHT_THUMB,    ControllerButton::RightStick);
                checkBtn(XINPUT_GAMEPAD_DPAD_UP,        ControllerButton::DpadUp);
                checkBtn(XINPUT_GAMEPAD_DPAD_DOWN,      ControllerButton::DpadDown);
                checkBtn(XINPUT_GAMEPAD_DPAD_LEFT,      ControllerButton::DpadLeft);
                checkBtn(XINPUT_GAMEPAD_DPAD_RIGHT,     ControllerButton::DpadRight);
                checkBtn(XINPUT_GAMEPAD_START,          ControllerButton::Start);
                checkBtn(XINPUT_GAMEPAD_BACK,           ControllerButton::Back);

                auto pushAxis = [&](ControllerAxis axis, float value) {
                    InputEvent e{};
                    e.type          = InputEventType::ControllerAxis;
                    e.time          = now;
                    e.ctrlAxis.id   = (uint8_t)i;
                    e.ctrlAxis.axis = axis;
                    e.ctrlAxis.value = value;
                    ring.push(e);
                };

                auto norm16 = [](SHORT v) { return v < 0
                    ? -(float)v / 32768.0f
                    :  (float)v / 32767.0f; };
                auto norm8  = [](BYTE  v) { return (float)v / 255.0f; };

                if (curr.sThumbLX != prev.sThumbLX)
                    pushAxis(ControllerAxis::LeftX, norm16(curr.sThumbLX));
                if (curr.sThumbLY != prev.sThumbLY)
                    pushAxis(ControllerAxis::LeftY, -norm16(curr.sThumbLY));
                if (curr.sThumbRX != prev.sThumbRX)
                    pushAxis(ControllerAxis::RightX, norm16(curr.sThumbRX));
                if (curr.sThumbRY != prev.sThumbRY)
                    pushAxis(ControllerAxis::RightY, -norm16(curr.sThumbRY));
                if (curr.bLeftTrigger  != prev.bLeftTrigger)
                    pushAxis(ControllerAxis::TriggerLeft, norm8(curr.bLeftTrigger));
                if (curr.bRightTrigger != prev.bRightTrigger)
                    pushAxis(ControllerAxis::TriggerRight, norm8(curr.bRightTrigger));

                lastState[i] = state;
            }
        }
    }
#endif