// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

#include <string>
#include <string_view>
#include <functional>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <format>
#include <chrono>
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <filesystem>
#include <emmintrin.h>

#ifdef _WIN32
    #include <Windows.h>
    #include <windowsx.h>
    #include <Xinput.h>
    #include <timeapi.h>
    #include <wrl/client.h>

    using Microsoft::WRL::ComPtr;
    #pragma comment(lib, "xinput.lib")
#endif