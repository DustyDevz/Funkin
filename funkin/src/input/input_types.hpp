// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <cstdint>
#include <string>

namespace Funkin::Input {
    using Timestamp = uint64_t;

    enum class KeyCode : uint32_t {
        Unknown = 0,

        // letters
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // numbers
        Num0, Num1, Num2, Num3, Num4,
        Num5, Num6, Num7, Num8, Num9,

        // numpad
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadAdd, NumpadSub, NumpadMul, NumpadDiv, NumpadEnter,

        // function keys
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        // modifiers
        LShift, RShift, LCtrl, RCtrl, LAlt, RAlt, LSuper, RSuper,

        // navigation
        Up, Down, Left, Right,
        Home, End, PageUp, PageDown,
        Insert, Delete,

        // special
        Enter, Escape, Space, Tab, Backspace, CapsLock,
        PrintScreen, ScrollLock, Pause,

        // punctuation
        Comma, Period, Slash, Backslash,
        Semicolon, Apostrophe, Grave,
        LBracket, RBracket, Minus, Equal,

        COUNT
    };

    enum class MouseButton : uint8_t {
        Left, Right, Middle, X1, X2
    };

    enum class ControllerButton : uint8_t {
        A, B, X, Y,
        LBumper, RBumper,
        LTrigger, RTrigger,
        LStick, RStick,
        DPadUp, DPadDown, DPadLeft, DPadRight,
        Start, Back, Guide,
        COUNT
    };

    enum class ControllerAxis : uint8_t {
        LX, LY, // left stick
        RX, RY, // right stick
        LT, RT, // triggers
        COUNT
    };

    enum class InputEventType : uint8_t {
        KeyDown,
        KeyUp,
        MouseMove,
        MouseDown,
        MouseUp,
        MouseScroll,
        ControllerButtonDown,
        ControllerButtonUp,
        ControllerAxis,
        ControllerConnected,
        ControllerDisconnected,
    };

    struct InputEvent {
        InputEventType type;
        Timestamp      time;

        union {
            // keyboard
            struct { KeyCode key; bool repeat; } key;

            // mouse
            struct { float x, y; }               mouseMove;
            struct { MouseButton btn; }          mouseButton;
            struct { float dx, dy; }             mouseScroll;

            // controller
            struct { uint8_t id; ControllerButton btn; } ctrlBtn;
            struct { uint8_t id; ControllerAxis   axis; float value; } ctrlAxis;
            struct { uint8_t id; }               ctrlConnect;
        };
    };

    const char* keyCodeName(KeyCode k);
    KeyCode     keyCodeFromName(const char* name);
}