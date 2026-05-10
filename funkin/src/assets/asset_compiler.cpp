// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "asset_compiler.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Funkin::Assets {
    AssetFingerprint AssetCompiler::fingerprint(const std::filesystem::path& path) {
        AssetFingerprint fp{};
        std::error_code ec;
        auto lwt  = std::filesystem::last_write_time(path, ec);
        fp.mtime  = ec ? 0 : (uint64_t)lwt.time_since_epoch().count();
        fp.fileSize = ec ? 0 : (uint64_t)std::filesystem::file_size(path, ec);
        return fp;
    }

    std::vector<uint8_t> AssetCompiler::loadRaw(const std::filesystem::path& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open: " + path.string());
        return { std::istreambuf_iterator<char>(f), {} };
    }

    static AssetSubtype shaderStageToSubtype(bool isVertex) {
        return isVertex ? AssetSubtype::ShaderVertex : AssetSubtype::ShaderPixel;
    }

    std::vector<CompiledAsset> AssetCompiler::compileShader(
        const std::filesystem::path& path, ShaderBackend backend)
    {
        std::vector<CompiledAsset> results;
        auto fp = fingerprint(path);

        auto compile = [&](const char* entry, AssetSubtype subtype) {
            #ifdef _WIN32
                if (backend == ShaderBackend::DX12) {
                    std::ifstream f(path);
                    std::string source((std::istreambuf_iterator<char>(f)), {});

                    const char* profile = (subtype == AssetSubtype::ShaderVertex) ? "vs_5_0" : "ps_5_0";
                    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
                    #ifdef FUNKIN_DEBUG
                        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
                    #else
                        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
                    #endif
                    ComPtr<ID3DBlob> blob, err;
                    HRESULT hr = D3DCompile(source.c_str(), source.size(),
                        path.string().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                        entry, profile, flags, 0, &blob, &err);

                    if (FAILED(hr)) {
                        std::string msg = "Shader compile error [" + path.string() + "]: ";
                        if (err) msg += (char*)err->GetBufferPointer();
                        throw std::runtime_error(msg);
                    }

                    CompiledAsset asset;
                    asset.type         = AssetType::Shader;
                    asset.subtype      = subtype;
                    asset.shaderBackend= ShaderBackend::DX12;
                    asset.compressMode = CompressMode::Max;
                    asset.fingerprint  = fp;
                    asset.sourcePath   = path.string();
                    asset.data.assign((uint8_t*)blob->GetBufferPointer(),
                                    (uint8_t*)blob->GetBufferPointer() + blob->GetBufferSize());

                    std::string stem = path.stem().string();
                    asset.id = makeShaderID(stem, subtype);
                    results.push_back(std::move(asset));
                }
            #endif
            if (backend == ShaderBackend::Vulkan && path.extension() == ".spv") {
                CompiledAsset asset;
                asset.type         = AssetType::Shader;
                asset.subtype      = subtype;
                asset.shaderBackend= ShaderBackend::Vulkan;
                asset.compressMode = CompressMode::Max;
                asset.fingerprint  = fp;
                asset.sourcePath   = path.string();
                asset.data         = loadRaw(path);
                asset.id           = makeShaderID(path.stem().string(), subtype);
                results.push_back(std::move(asset));
            }
        };

        if (path.extension() == ".hlsl") {
            compile("VSMain", AssetSubtype::ShaderVertex);
            compile("PSMain", AssetSubtype::ShaderPixel);
        } else if (path.extension() == ".spv") {
            compile(nullptr, AssetSubtype::ShaderVertex);
        }

        return results;
    }

    CompiledAsset AssetCompiler::compileTexture(const std::filesystem::path& path) {
        CompiledAsset asset;
        asset.type        = AssetType::Texture;
        asset.subtype     = inferAssetSubtype(path.extension().string().substr(1));
        asset.compressMode= pickCompressMode(asset.type, asset.subtype);
        asset.fingerprint = fingerprint(path);
        asset.sourcePath  = path.string();
        asset.id          = makeAssetID(path.string());

        auto ext = path.extension().string();
        if (ext == ".dds") {
            asset.data = loadRaw(path);
            return asset;
        }

        int w, h, ch;
        stbi_set_flip_vertically_on_load(false);
        uint8_t* px = stbi_load(path.string().c_str(), &w, &h, &ch, 4);
        if (!px) throw std::runtime_error("stb_image failed: " + path.string());

        asset.data.reserve(12 + (size_t)w * h * 4);
        auto push32 = [&](uint32_t v) {
            asset.data.push_back( v        & 0xFF);
            asset.data.push_back((v >>  8) & 0xFF);
            asset.data.push_back((v >> 16) & 0xFF);
            asset.data.push_back((v >> 24) & 0xFF);
        };
        push32((uint32_t)w);
        push32((uint32_t)h);
        push32(4u);
        asset.data.insert(asset.data.end(), px, px + (size_t)w * h * 4);
        stbi_image_free(px);
        return asset;
    }

    CompiledAsset AssetCompiler::compileAudio(const std::filesystem::path& path) {
        CompiledAsset asset;
        asset.type        = AssetType::Audio;
        asset.subtype     = inferAssetSubtype(path.extension().string().substr(1));
        asset.compressMode= pickCompressMode(asset.type, asset.subtype);
        asset.fingerprint = fingerprint(path);
        asset.sourcePath  = path.string();
        asset.id          = makeAssetID(path.string());
        asset.data        = loadRaw(path);
        return asset;
    }

    CompiledAsset AssetCompiler::compileFont(const std::filesystem::path& path) {
        CompiledAsset asset;
        asset.type        = AssetType::Font;
        asset.subtype     = AssetSubtype::None;
        asset.compressMode= CompressMode::Max;
        asset.fingerprint = fingerprint(path);
        asset.sourcePath  = path.string();
        asset.id          = makeAssetID(path.string());
        asset.data        = loadRaw(path);
        return asset;
    }

    CompiledAsset AssetCompiler::compileBinary(const std::filesystem::path& path) {
        CompiledAsset asset;
        asset.type        = AssetType::Binary;
        asset.subtype     = AssetSubtype::None;
        asset.compressMode= CompressMode::Max;
        asset.fingerprint = fingerprint(path);
        asset.sourcePath  = path.string();
        asset.id          = makeAssetID(path.string());
        asset.data        = loadRaw(path);
        return asset;
    }

    CompiledAsset AssetCompiler::compileFile(const std::filesystem::path& path,
                                               ShaderBackend shaderBackend) {
        auto ext  = path.extension().string();
        auto type = inferAssetType(ext.empty() ? "" : ext.substr(1));

        if (type == AssetType::Shader) {
            auto shaders = compileShader(path, shaderBackend);
            if (!shaders.empty()) return shaders[0];
            throw std::runtime_error("No shader output: " + path.string());
        }
        if (type == AssetType::Texture) return compileTexture(path);
        if (type == AssetType::Audio)   return compileAudio(path);
        if (type == AssetType::Font)    return compileFont(path);
        return compileBinary(path);
    }

    std::vector<CompiledAsset> AssetCompiler::compileFolder(
        const std::filesystem::path& folder,
        const std::vector<AssetTOCEntry>& knownEntries,
        ShaderBackend shaderBackend)
    {
        std::unordered_map<AssetID, AssetFingerprint, AssetIDHasher> known;
        for (auto& e : knownEntries)
            known[e.id] = e.fingerprint;

        std::vector<CompiledAsset> results;

        for (auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
            if (!entry.is_regular_file()) continue;
            auto& p   = entry.path();
            auto  ext = p.extension().string();
            if (ext == ".fkpak" || ext == ".funkin") continue;

            try {
                auto type = inferAssetType(ext.empty() ? "" : ext.substr(1));

                if (type == AssetType::Shader) {
                    auto stem = p.stem().string();
                    auto fp   = fingerprint(p);

                    bool vsStale = true, psStale = true;
                    auto vsID = makeShaderID(stem, AssetSubtype::ShaderVertex);
                    auto psID = makeShaderID(stem, AssetSubtype::ShaderPixel);

                    auto vit = known.find(vsID);
                    if (vit != known.end() &&
                        vit->second.mtime == fp.mtime &&
                        vit->second.fileSize == fp.fileSize)
                        vsStale = false;

                    auto pit = known.find(psID);
                    if (pit != known.end() &&
                        pit->second.mtime == fp.mtime &&
                        pit->second.fileSize == fp.fileSize)
                        psStale = false;

                    if (vsStale || psStale) {
                        auto shaders = compileShader(p, shaderBackend);
                        for (auto& s : shaders) {
                            bool isVS = s.subtype == AssetSubtype::ShaderVertex;
                            if ((isVS && vsStale) || (!isVS && psStale))
                                results.push_back(std::move(s));
                        }
                    }
                    continue;
                }

                AssetID id  = makeAssetID(p.string());
                auto    fp  = fingerprint(p);
                auto    kit = known.find(id);

                if (kit != known.end() &&
                    kit->second.mtime    == fp.mtime &&
                    kit->second.fileSize == fp.fileSize)
                    continue;

                if (type == AssetType::Texture) results.push_back(compileTexture(p));
                else if (type == AssetType::Audio)  results.push_back(compileAudio(p));
                else if (type == AssetType::Font)   results.push_back(compileFont(p));
                else                                results.push_back(compileBinary(p));

            } catch (const std::exception& e) {
                fprintf(stderr, "[AssetCompiler] SKIP %s: %s\n",
                        p.string().c_str(), e.what());
            }
        }

        return results;
    }
}