#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <format>
#include <chrono>
#include <iostream>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
    #include <wrl/client.h>
    #include <directx/d3d12.h>
    #include <directx/d3dx12.h>
    #include <dxgi1_6.h>
    using Microsoft::WRL::ComPtr;
#endif
