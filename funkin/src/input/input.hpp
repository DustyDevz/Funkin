// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "input_types.hpp"
#include "input_ring_buffer.hpp"

namespace Funkin::Input {
    struct Binding {
        std::string      action;
        KeyCode          key        = KeyCode::Unknown;
        ControllerButton ctrlBtn    = ControllerButton::COUNT;
        MouseButton      mouseBtn   = MouseButton::Left;
        bool             useKey     = false;
        bool             useCtrl    = false;
        bool             useMouse   = false;
    };

    struct DeadZone {
        float inner = 0.15f;
        float outer = 0.95f;
    };

    struct AxisState {
        float raw       = 0.0f;
        float processed = 0.0f;
    };

    struct InputState {
        // keyboard
        std::array<bool, (size_t)KeyCode::COUNT>          keys{};
        std::array<bool, (size_t)KeyCode::COUNT>          keysJustDown{};
        std::array<bool, (size_t)KeyCode::COUNT>          keysJustUp{};

        // mouse
        float mouseX = 0, mouseY = 0;
        float mouseDX = 0, mouseDY = 0;
        float scrollX = 0, scrollY = 0;
        std::array<bool, 5> mouseButtons{};
        std::array<bool, 5> mouseJustDown{};
        std::array<bool, 5> mouseJustUp{};

        // controller [4 max!!]
        struct ControllerState {
            bool connected = false;
            std::array<bool, (size_t)ControllerButton::COUNT> buttons{};
            std::array<bool, (size_t)ControllerButton::COUNT> justDown{};
            std::array<bool, (size_t)ControllerButton::COUNT> justUp{};
            std::array<AxisState, (size_t)ControllerAxis::COUNT> axes{};
        };

        std::array<ControllerState, 4> controllers{};
        std::vector<InputEvent> frameEvents;
    };

    class Input {
    public:
        static Input& get();

        void init();
        void shutdown();
        void update();
        void syncMousePosition();

        const InputState& state() const { return m_state; }

        bool isDown(KeyCode k)      const { return m_state.keys[(size_t)k]; }
        bool justDown(KeyCode k)    const { return m_state.keysJustDown[(size_t)k]; }
        bool justUp(KeyCode k)      const { return m_state.keysJustUp[(size_t)k]; }

        bool isDown(const std::string& action)   const;
        bool justDown(const std::string& action) const;
        bool justUp(const std::string& action)   const;

        float axis(uint8_t controller, ControllerAxis a) const;
        const std::vector<InputEvent>& frameEvents() const { return m_state.frameEvents; }

        void   bind(const std::string& action, KeyCode key);
        void   bind(const std::string& action, ControllerButton btn);
        void   unbind(const std::string& action);
        void   loadBindings(const std::string& path);
        void   saveBindings(const std::string& path) const;

        void     setDeadZone(ControllerAxis axis, DeadZone dz);
        DeadZone deadZone(ControllerAxis axis) const;

        InputRingBuffer<4096>& ring()            { return m_ring; }
        uint64_t               startTime() const { return m_startTime; }

        uint64_t getNow() const;
        uint64_t getLastTimestamp(const std::string& action) const;

    private:
        Input() = default;

        void inputThreadFunc();
        void pollControllers();
        float applyDeadZone(float value, const DeadZone& dz) const;

        static constexpr size_t RING_SIZE = 4096;
        InputRingBuffer<RING_SIZE> m_ring;

        InputState m_state;
        InputState m_prev;

        std::unordered_map<std::string, Binding> m_bindings;
        std::array<DeadZone, (size_t)ControllerAxis::COUNT> m_deadZones{};
        std::thread       m_inputThread;
        std::atomic<bool> m_threadRunning{ false };
        uint64_t m_startTime = 0;

        void* m_windowHandle = nullptr;
    };
}