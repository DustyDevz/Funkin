// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "shader_cache.hpp"

namespace Funkin::Renderer::Shader {
    std::string ShaderCache::makeKey(const std::string& name, ShaderStage stage) const {
        return name + "_" + std::to_string((int)stage);
    }

    void ShaderCache::load(ShaderPack&& pack) {
        for (auto& s : pack.shaders)
            m_cache[makeKey(s.name, s.stage)] = std::move(s);
    }

    void ShaderCache::clear() {
        m_cache.clear();
    }

    const ShaderCode* ShaderCache::get(const std::string& name, ShaderStage stage) const {
        auto it = m_cache.find(makeKey(name, stage));
        return it != m_cache.end() ? &it->second : nullptr;
    }

    bool ShaderCache::has(const std::string& name, ShaderStage stage) const {
        return m_cache.count(makeKey(name, stage)) > 0;
    }
}