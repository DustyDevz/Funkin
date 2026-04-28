// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "shader_loader.hpp"
#include <filesystem>
#include <stdexcept>

namespace Funkin::Renderer::Shader {
    ShaderLoader& ShaderLoader::get() {
        static ShaderLoader s;
        return s;
    }

    void ShaderLoader::init(ShaderBackend backend) {
        m_backend = backend;

        if (std::filesystem::exists(PACK_PATH)) {
            auto pack = ShaderPacker::unpack(PACK_PATH);
            m_cache.load(std::move(pack));

        } else if (std::filesystem::exists(SHADER_PATH)) {
            auto pack = ShaderCompiler::compileFolder(SHADER_PATH, backend);
            ShaderPacker::pack(pack, PACK_PATH);
            m_cache.load(std::move(pack));
        } else {
            throw std::runtime_error("No shaders found in bin/shaders.funkin or shaders/");
        }
    }

    void ShaderLoader::shutdown() {
        m_cache.clear();
    }

    const ShaderCode* ShaderLoader::get(const std::string& name, ShaderStage stage) const {
        return m_cache.get(name, stage);
    }
}