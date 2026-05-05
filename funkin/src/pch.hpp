#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
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

#include <math/vec2.hpp>
#include <math/vec3.hpp>
#include <math/vec4.hpp>
#include <math/color.hpp>
#include <math/rect.hpp>
#include <math/mat4.hpp>

#include <vulkan/vulkan.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <stb_truetype.h>
    #include <Windows.h>
    #include <wrl/client.h>
    #include <directx/d3d12.h>
    #include <directx/d3dx12.h>
    #include <dxgi1_6.h>
    using Microsoft::WRL::ComPtr;
#endif