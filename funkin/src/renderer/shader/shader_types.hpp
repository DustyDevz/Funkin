// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace Funkin::Renderer::Shader {
    enum class ShaderStage {
        Vertex,
        Pixel,    // dx12
        Fragment, // vk
        Compute
    };

    enum class ShaderBackend {
        DX12,
        Vulkan,
    };

    struct ShaderCode {
        std::vector<uint8_t> bytecode; // dx12 - dxil / vk - spir-v
        ShaderStage          stage;
        ShaderBackend        backend;
        std::string          name;

        bool empty() const { return bytecode.empty(); }
    };

    struct ShaderPack {
        std::vector<ShaderCode> shaders;

        const ShaderCode* find(const std::string& name, ShaderStage stage) const {
            for (auto& s : shaders)
                if (s.name == name && s.stage == stage)
                    return &s;
                    
            return nullptr;
        }
    };
}