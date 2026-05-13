// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
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

#include <math/math_vec2.hpp>
#include <math/math_vec3.hpp>
#include <math/math_vec4.hpp>
#include <math/math_color.hpp>
#include <math/math_rect.hpp>
#include <math/math_mat4.hpp>

#include <assets/asset_types.hpp>
#include <assets/asset_handle.hpp>
#include <assets/asset_loader.hpp>
#include <assets/asset_cache.hpp>
#include <assets/asset_compiler.hpp>
#include <assets/asset_packer.hpp>
#include <assets/asset_watcher.hpp>

#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#ifdef _WIN32
    #include <Windows.h>
    #include <windowsx.h>
    #include <Xinput.h>
    #include <timeapi.h>
    #include <wrl/client.h>
    
    #include <directx/d3dx12.h>
    #include <d3d12.h>
    #include <dxgi1_6.h>
    #include <d3d11on12.h>
    #include <d2d1_3.h>
    #include <dwrite_3.h>
    #include <d3dcompiler.h>
    
    #include <backends/imgui_impl_win32.h>
    #include <backends/imgui_impl_dx12.h>

    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "d2d1.lib")
    #pragma comment(lib, "dwrite.lib")
    #pragma comment(lib, "d3dcompiler.lib")
    #pragma comment(lib, "winmm.lib")
    #pragma comment(lib, "xinput.lib")

    using Microsoft::WRL::ComPtr;
#else
    #include <backends/imgui_impl_glfw.h>
#endif

#include <backends/imgui_impl_vulkan.h>
#include <shared/log.hpp>