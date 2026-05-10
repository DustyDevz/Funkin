// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"
#include "asset_compiler.hpp"
#include <filesystem>
#include <vector>

namespace Funkin::Assets {

    class AssetPacker {
    public:
        static std::vector<AssetTOCEntry> readTOC(const std::filesystem::path& path);
        static uint32_t update(const std::filesystem::path& packPath,
                               const std::vector<CompiledAsset>& newAssets,
                               const std::vector<AssetID>& removedIDs = {});

        static void packAll(const std::filesystem::path& packPath,
                            const std::vector<CompiledAsset>& assets);

        static void     list(const std::filesystem::path& packPath);
        static void     extract(const std::filesystem::path& packPath,
                                std::string_view assetPath,
                                const std::filesystem::path& outDir);
        static void     extractAll(const std::filesystem::path& packPath,
                                   const std::filesystem::path& outDir);
        static void     remove(const std::filesystem::path& packPath,
                               std::string_view assetPath);
        static void     addFile(const std::filesystem::path& packPath,
                                const std::filesystem::path& filePath,
                                ShaderBackend backend = ShaderBackend::DX12);
        static void     rebuild(const std::filesystem::path& packPath);

    private:
        static std::vector<uint8_t> compress(const std::vector<uint8_t>& src, int level);
        static std::vector<uint8_t> decompress(const std::vector<uint8_t>& src,
                                                uint32_t originalSize, uint64_t seed);
        static void obfuscate(std::vector<uint8_t>& data, uint64_t seed);
        static void deobfuscate(std::vector<uint8_t>& data, uint64_t seed);

        static void writePack(const std::filesystem::path& path,
                              const std::vector<AssetTOCEntry>& toc,
                              const std::vector<std::vector<uint8_t>>& blobs);
    };
}