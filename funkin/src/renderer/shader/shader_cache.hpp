// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "shader_types.hpp"
#include <unordered_map>
#include <string>

namespace Funkin::Renderer::Shader {
    class ShaderCache {
    public:
        void load(ShaderPack&& pack);
        void clear();

        const ShaderCode* get(const std::string& name, ShaderStage stage) const;
        bool              has(const std::string& name, ShaderStage stage) const;
        
    private:
        std::string makeKey(const std::string& name, ShaderStage stage) const;
        std::unordered_map<std::string, ShaderCode> m_cache;
    };
}