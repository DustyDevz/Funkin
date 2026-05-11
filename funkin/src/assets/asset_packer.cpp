// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "asset_packer.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <zstd.h>
#include <unordered_map>

namespace Funkin::Assets {
    static uint64_t nowUnix() {
        return (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::vector<uint8_t> AssetPacker::compress(const std::vector<uint8_t>& src, int level) {
        if (level == 0) return src;
        size_t bound = ZSTD_compressBound(src.size());
        std::vector<uint8_t> dst(bound);
        size_t result = ZSTD_compress(dst.data(), bound, src.data(), src.size(), level);
        if (ZSTD_isError(result))
            throw std::runtime_error(std::string("zstd compress: ") + ZSTD_getErrorName(result));
        dst.resize(result);
        return dst;
    }

    std::vector<uint8_t> AssetPacker::decompress(const std::vector<uint8_t>& src,
                                                   uint32_t originalSize, uint64_t seed) {
        auto tmp = src;
        deobfuscate(tmp, seed);
        std::vector<uint8_t> dst(originalSize);
        size_t result = ZSTD_decompress(dst.data(), originalSize, tmp.data(), tmp.size());
        if (ZSTD_isError(result))
            throw std::runtime_error(std::string("zstd decompress: ") + ZSTD_getErrorName(result));
        return dst;
    }

    void AssetPacker::obfuscate(std::vector<uint8_t>& data, uint64_t seed) {
        uint8_t key = XOR_KEY ^ (uint8_t)(seed & 0xFF) ^ XOR_SALT;
        for (size_t i = 0; i < data.size(); ++i)
            data[i] ^= (key + (uint8_t)(i & 0xFF));
    }

    void AssetPacker::deobfuscate(std::vector<uint8_t>& data, uint64_t seed) {
        obfuscate(data, seed);
    }

    void AssetPacker::writePack(const std::filesystem::path& path,
                                 const std::vector<AssetTOCEntry>& toc,
                                 const std::vector<std::vector<uint8_t>>& blobs) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f) throw std::runtime_error("Cannot write pack: " + path.string());

        AssetPackHeader hdr{};
        hdr.magic      = FUNKIN_MAGIC;
        hdr.version    = FUNKIN_VERSION;
        hdr.entryCount = (uint32_t)toc.size();
        hdr.buildTime  = nowUnix();
        hdr.tocOffset  = 0;

        f.write((char*)&hdr, sizeof(hdr));

        std::vector<AssetTOCEntry> finalTOC = toc;
        for (size_t i = 0; i < blobs.size(); ++i) {
            finalTOC[i].offset = (uint64_t)f.tellp();
            f.write((char*)blobs[i].data(), blobs[i].size());
        }

        hdr.tocOffset = (uint64_t)f.tellp();
        f.write((char*)finalTOC.data(), finalTOC.size() * sizeof(AssetTOCEntry));
        f.seekp(offsetof(AssetPackHeader, tocOffset));
        f.write((char*)&hdr.tocOffset, sizeof(hdr.tocOffset));
    }

    void AssetPacker::packAll(const std::filesystem::path& packPath,
                               const std::vector<CompiledAsset>& assets) {
        std::vector<AssetTOCEntry>        toc;
        std::vector<std::vector<uint8_t>> blobs;
        toc.reserve(assets.size());
        blobs.reserve(assets.size());

        for (auto& asset : assets) {
            int level = compressModeToZstdLevel(asset.compressMode);
            auto compressed = compress(asset.data, level);
            if (asset.compressMode != CompressMode::None)
                obfuscate(compressed, asset.id.hash);

            AssetTOCEntry entry{};
            entry.id             = asset.id;
            entry.type           = asset.type;
            entry.subtype        = asset.subtype;
            entry.compressMode   = asset.compressMode;
            entry.shaderBackend  = asset.shaderBackend;
            entry.offset         = 0;
            entry.compressedSize = (uint32_t)compressed.size();
            entry.originalSize   = (uint32_t)asset.data.size();
            entry.fingerprint    = asset.fingerprint;
            strncpy(entry.path, asset.sourcePath.c_str(), 255);

            toc.push_back(entry);
            blobs.push_back(std::move(compressed));
        }

        writePack(packPath, toc, blobs);
    }

    uint32_t AssetPacker::update(const std::filesystem::path& packPath,
                                  const std::vector<CompiledAsset>& newAssets,
                                  const std::vector<AssetID>& removedIDs) {
        std::vector<AssetTOCEntry>        existingTOC;
        std::vector<std::vector<uint8_t>> existingBlobs;

        if (std::filesystem::exists(packPath)) {
            existingTOC = readTOC(packPath);
            std::ifstream f(packPath, std::ios::binary);
            existingBlobs.resize(existingTOC.size());
            for (size_t i = 0; i < existingTOC.size(); ++i) {
                auto& e = existingTOC[i];
                f.seekg(e.offset);
                existingBlobs[i].resize(e.compressedSize);
                f.read((char*)existingBlobs[i].data(), e.compressedSize);
            }
        }

        std::unordered_map<AssetID, size_t, AssetIDHasher> idxMap;
        for (size_t i = 0; i < existingTOC.size(); ++i)
            idxMap[existingTOC[i].id] = i;

        for (auto& rid : removedIDs) {
            auto it = idxMap.find(rid);
            if (it == idxMap.end()) continue;
            size_t idx = it->second;
            existingTOC.erase(existingTOC.begin() + idx);
            existingBlobs.erase(existingBlobs.begin() + idx);
            idxMap.clear();
            for (size_t i = 0; i < existingTOC.size(); ++i)
                idxMap[existingTOC[i].id] = i;
        }

        uint32_t changed = 0;

        for (auto& asset : newAssets) {
            int level = compressModeToZstdLevel(asset.compressMode);
            auto compressed = compress(asset.data, level);
            if (asset.compressMode != CompressMode::None)
                obfuscate(compressed, asset.id.hash);

            AssetTOCEntry entry{};
            entry.id             = asset.id;
            entry.type           = asset.type;
            entry.subtype        = asset.subtype;
            entry.compressMode   = asset.compressMode;
            entry.shaderBackend  = asset.shaderBackend;
            entry.offset         = 0;
            entry.compressedSize = (uint32_t)compressed.size();
            entry.originalSize   = (uint32_t)asset.data.size();
            entry.fingerprint    = asset.fingerprint;
            strncpy(entry.path, asset.sourcePath.c_str(), 255);

            auto it = idxMap.find(asset.id);
            if (it != idxMap.end()) {
                size_t idx = it->second;
                existingTOC[idx]   = entry;
                existingBlobs[idx] = std::move(compressed);
            } else {
                idxMap[asset.id] = existingTOC.size();
                existingTOC.push_back(entry);
                existingBlobs.push_back(std::move(compressed));
            }
            ++changed;
        }

        writePack(packPath, existingTOC, existingBlobs);
        return changed;
    }

    std::vector<AssetTOCEntry> AssetPacker::readTOC(const std::filesystem::path& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open pack: " + path.string());

        AssetPackHeader hdr{};
        f.read((char*)&hdr, sizeof(hdr));
        if (hdr.magic != FUNKIN_MAGIC)
            throw std::runtime_error("Invalid .funkin pack file");
        if (hdr.version != FUNKIN_VERSION)
            throw std::runtime_error("Pack version mismatch");

        f.seekg(hdr.tocOffset);
        std::vector<AssetTOCEntry> toc(hdr.entryCount);
        f.read((char*)toc.data(), hdr.entryCount * sizeof(AssetTOCEntry));
        return toc;
    }

    void AssetPacker::list(const std::filesystem::path& packPath) {
        auto toc = readTOC(packPath);

        printf("%-60s %-10s %-12s %-12s %-8s\n",
               "Path", "Type", "Original", "Compressed", "Ratio");
        printf("%s\n", std::string(110, '-').c_str());

        uint64_t totalOrig = 0, totalComp = 0;
        for (auto& e : toc) {
            const char* typeName = "unknown";
            switch (e.type) {
                case AssetType::Texture: typeName = "texture"; break;
                case AssetType::Audio:   typeName = "audio";   break;
                case AssetType::Font:    typeName = "font";    break;
                case AssetType::Shader:  typeName = "shader";  break;
                case AssetType::Binary:  typeName = "binary";  break;
                default: break;
            }
            float ratio = e.originalSize > 0
                ? (1.0f - (float)e.compressedSize / (float)e.originalSize) * 100.0f
                : 0.0f;
            printf("%-60s %-10s %-12u %-12u %.1f%%\n",
                   e.path, typeName, e.originalSize, e.compressedSize, ratio);
            totalOrig += e.originalSize;
            totalComp += e.compressedSize;
        }
        printf("%s\n", std::string(110, '-').c_str());
        float totalRatio = totalOrig > 0
            ? (1.0f - (float)totalComp / (float)totalOrig) * 100.0f : 0.0f;
        printf("Total: %llu bytes → %llu bytes (%.1f%% saved)\n",
               (unsigned long long)totalOrig,
               (unsigned long long)totalComp, totalRatio);
    }

    void AssetPacker::extract(const std::filesystem::path& packPath,
                               std::string_view assetPath,
                               const std::filesystem::path& outDir) {
        auto toc = readTOC(packPath);
        AssetID target = makeAssetID(assetPath);

        for (auto& e : toc) {
            if (e.id != target) continue;

            std::ifstream f(packPath, std::ios::binary);
            f.seekg(e.offset);
            std::vector<uint8_t> compressed(e.compressedSize);
            f.read((char*)compressed.data(), e.compressedSize);

            std::vector<uint8_t> data;
            if (e.compressMode == CompressMode::None) {
                data = compressed;
            } else {
                deobfuscate(compressed, e.id.hash);
                data.resize(e.originalSize);
                ZSTD_decompress(data.data(), e.originalSize,
                                compressed.data(), compressed.size());
            }

            std::filesystem::path out = outDir / std::filesystem::path(e.path).filename();
            std::filesystem::create_directories(out.parent_path());
            std::ofstream of(out, std::ios::binary);
            of.write((char*)data.data(), data.size());
            printf("Extracted: %s -> %s\n", e.path, out.string().c_str());
            return;
        }
        fprintf(stderr, "Asset not found: %.*s\n", (int)assetPath.size(), assetPath.data());
    }

    void AssetPacker::extractAll(const std::filesystem::path& packPath,
                                  const std::filesystem::path& outDir) {
        auto toc = readTOC(packPath);
        std::ifstream f(packPath, std::ios::binary);

        for (auto& e : toc) {
            f.seekg(e.offset);
            std::vector<uint8_t> compressed(e.compressedSize);
            f.read((char*)compressed.data(), e.compressedSize);

            std::vector<uint8_t> data;
            if (e.compressMode == CompressMode::None) {
                data = compressed;
            } else {
                deobfuscate(compressed, e.id.hash);
                data.resize(e.originalSize);
                ZSTD_decompress(data.data(), e.originalSize,
                                compressed.data(), compressed.size());
            }

            std::filesystem::path rel  = std::filesystem::path(e.path);
            std::filesystem::path out  = outDir / rel.filename();
            std::filesystem::create_directories(out.parent_path());
            std::ofstream of(out, std::ios::binary);
            of.write((char*)data.data(), data.size());
            printf("Extracted: %s\n", out.string().c_str());
        }
    }

    void AssetPacker::remove(const std::filesystem::path& packPath,
                              std::string_view assetPath) {
        AssetID id = makeAssetID(assetPath);
        update(packPath, {}, { id });
        printf("Removed: %.*s\n", (int)assetPath.size(), assetPath.data());
    }

    void AssetPacker::addFile(const std::filesystem::path& packPath,
                               const std::filesystem::path& filePath,
                               ShaderBackend backend) {
        auto ext  = filePath.extension().string();
        auto type = inferAssetType(ext.empty() ? "" : ext.substr(1));

        std::vector<CompiledAsset> assets;
        if (type == AssetType::Shader) {
            assets = AssetCompiler::compileFolder(filePath.parent_path(), {}, backend);
        } else {
            assets.push_back(AssetCompiler::compileFile(filePath, backend));
        }
        uint32_t n = update(packPath, assets);
        printf("Added/updated %u asset(s) from %s\n", n, filePath.string().c_str());
    }

    void AssetPacker::rebuild(const std::filesystem::path& packPath) {
        auto toc = readTOC(packPath);
        std::ifstream f(packPath, std::ios::binary);

        std::vector<CompiledAsset> assets;
        for (auto& e : toc) {
            f.seekg(e.offset);
            std::vector<uint8_t> compressed(e.compressedSize);
            f.read((char*)compressed.data(), e.compressedSize);

            CompiledAsset a;
            a.id           = e.id;
            a.type         = e.type;
            a.subtype      = e.subtype;
            a.compressMode = e.compressMode;
            a.shaderBackend= e.shaderBackend;
            a.fingerprint  = e.fingerprint;
            a.sourcePath   = e.path;

            if (e.compressMode == CompressMode::None) {
                a.data = compressed;
            } else {
                deobfuscate(compressed, e.id.hash);
                a.data.resize(e.originalSize);
                ZSTD_decompress(a.data.data(), e.originalSize,
                                compressed.data(), compressed.size());
            }
            assets.push_back(std::move(a));
        }

        packAll(packPath, assets);
        printf("Rebuilt: %s (%zu assets)\n", packPath.string().c_str(), assets.size());
    }
}