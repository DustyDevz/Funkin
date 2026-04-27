// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

namespace Funkin::Renderer {
    // dx12
    inline constexpr int FRAME_COUNT = 2;

    // vk
    inline void vkCheck(VkResult r, const char* msg) {
    if (r != VK_SUCCESS)
        throw std::runtime_error(msg);
    }
}