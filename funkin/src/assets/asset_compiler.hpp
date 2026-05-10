// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"
#include <filesystem>
#include <vector>

namespace Funkin::Assets {
    struct CompiledAsset {
        AssetID          id;
        AssetType        type;
        AssetSubtype     subtype;
        CompressMode     compressMode;
        ShaderBackend    shaderBackend = ShaderBackend::DX12;
        AssetFingerprint fingerprint;
        std::string      sourcePath;
        std::vector<uint8_t> data;
    };

    class AssetCompiler {
    public:
        static CompiledAsset compileFile(const std::filesystem::path& path,
                                          ShaderBackend shaderBackend = ShaderBackend::DX12);

        static std::vector<CompiledAsset> compileFolder(
            const std::filesystem::path& folder,
            const std::vector<AssetTOCEntry>& knownEntries = {},
            ShaderBackend shaderBackend = ShaderBackend::DX12);

        static AssetFingerprint fingerprint(const std::filesystem::path& path);

    private:
        static std::vector<uint8_t> loadRaw(const std::filesystem::path& path);
        static std::vector<CompiledAsset> compileShader(const std::filesystem::path& path,
                                                          ShaderBackend backend);
        static CompiledAsset compileTexture(const std::filesystem::path& path);
        static CompiledAsset compileAudio(const std::filesystem::path& path);
        static CompiledAsset compileFont(const std::filesystem::path& path);
        static CompiledAsset compileBinary(const std::filesystem::path& path);
    };
}