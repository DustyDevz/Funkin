// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma ONCE

#include "shader_types.hpp"
#include "shader_cache.hpp"
#include "shader_compiler.hpp"
#include "shader_packer.hpp"

namespace Funkin::Renderer::Shader {
    class ShaderLoader {
    public:
        static ShaderLoader& get();

        void init(ShaderBackend backend);
        void shutdown();

        const ShaderCode* get(const std::string& name, ShaderStage stage) const;

    private:
        ShaderLoader() = default;

        ShaderCache   m_cache;
        ShaderBackend m_backend = ShaderBackend::DX12;

        static constexpr const char* PACK_PATH   = "bin/shaders.funkin";
        static constexpr const char* SHADER_PATH = "shaders/";
    };
}