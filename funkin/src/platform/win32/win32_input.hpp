// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#ifdef _WIN32
    #include <windows.h>
    #include <xinput.h>
    #include <cstdint>
    #include "input/input_types.hpp"
    #include "input/input_ring_buffer.hpp"

    using namespace Funkin::Input;
    namespace Funkin::Platform::Input {
        KeyCode vkToKeyCode(WPARAM vk, LPARAM flags);

        void handleRawInput(HRAWINPUT hRaw,
                            InputRingBuffer<4096>& ring,
                            uint64_t startTime,
                            uint64_t capturedNow);

        void registerRawInput(HWND hwnd);
        void pollXInput(InputRingBuffer<4096>& ring,
                        uint64_t startTime,
                        XINPUT_STATE lastState[4],
                        bool connected[4]);
                        
        uint64_t nanoTime();
    }
#endif