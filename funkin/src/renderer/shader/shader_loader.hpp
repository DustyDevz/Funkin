// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "shader_types.hpp"

namespace Funkin::Renderer::Shader {

    class ShaderLoader {
    public:
        static ShaderLoader& get();
        void init(ShaderBackend backend);
        void shutdown();
        const ShaderCode* get(const std::string& name, ShaderStage stage) const;

    private:
        ShaderLoader() = default;

        Assets::AssetSubtype stageToSubtype(ShaderStage stage) const;
        ShaderStage          subtypeToStage(Assets::AssetSubtype sub) const;

        ShaderBackend m_backend = ShaderBackend::DX12;
        mutable std::unordered_map<std::string, ShaderCode> m_cache;
    };
}