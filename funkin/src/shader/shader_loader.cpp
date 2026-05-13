// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "shader_loader.hpp"

namespace Funkin::Renderer::Shader {

    ShaderLoader& ShaderLoader::get() {
        static ShaderLoader s;
        return s;
    }

    Assets::AssetSubtype ShaderLoader::stageToSubtype(ShaderStage stage) const {
        switch (stage) {
            case ShaderStage::Vertex:   return Assets::AssetSubtype::ShaderVertex;
            case ShaderStage::Pixel:    return Assets::AssetSubtype::ShaderPixel;
            case ShaderStage::Fragment: return Assets::AssetSubtype::ShaderFrag;
            case ShaderStage::Compute:  return Assets::AssetSubtype::ShaderCompute;
            default: return Assets::AssetSubtype::ShaderVertex;
        }
    }

    ShaderStage ShaderLoader::subtypeToStage(Assets::AssetSubtype sub) const {
        switch (sub) {
            case Assets::AssetSubtype::ShaderVertex:  return ShaderStage::Vertex;
            case Assets::AssetSubtype::ShaderPixel:   return ShaderStage::Pixel;
            case Assets::AssetSubtype::ShaderFrag:    return ShaderStage::Fragment;
            case Assets::AssetSubtype::ShaderCompute: return ShaderStage::Compute;
            default: return ShaderStage::Vertex;
        }
    }

    void ShaderLoader::init(ShaderBackend backend) {
        m_backend = backend;

        Assets::ShaderBackend assetBackend =
            (backend == ShaderBackend::DX12)
                ? Assets::ShaderBackend::DX12
                : Assets::ShaderBackend::Vulkan;

        auto& loader = Assets::AssetLoader::get();
        try {
            loader.init(PACK_PATH, SOURCE_PATH, assetBackend);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                std::string("[ShaderLoader] Asset system init failed: ") + e.what());
        }
    }

    void ShaderLoader::shutdown() {
        m_cache.clear();
    }

    const ShaderCode* ShaderLoader::get(const std::string& name, ShaderStage stage) const {
        std::string key = name + "_" + std::to_string((int)stage);

        auto it = m_cache.find(key);
        if (it != m_cache.end()) return &it->second;

        auto subtype = stageToSubtype(stage);
        Assets::ShaderHandle handle;

        try {
            handle = Assets::AssetLoader::get().loadShader(name, subtype);
        } catch (const std::exception& e) {
            fprintf(stderr, "[ShaderLoader] Failed to load shader '%s': %s\n",
                    name.c_str(), e.what());
            return nullptr;
        }

        if (!handle || handle->empty()) return nullptr;

        ShaderCode code;
        code.bytecode = handle->bytecode;
        code.stage    = stage;
        code.backend  = m_backend;
        code.name     = name;

        auto [ins, ok] = m_cache.emplace(key, std::move(code));
        return ok ? &ins->second : nullptr;
    }
}