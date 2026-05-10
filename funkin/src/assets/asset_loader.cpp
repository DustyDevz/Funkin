// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "asset_loader.hpp"
#include "asset_compiler.hpp"
#include <zstd.h>
#include <stdexcept>

namespace Funkin::Assets {
    AssetLoader& AssetLoader::get() {
        static AssetLoader s;
        return s;
    }

    void AssetLoader::init(const std::filesystem::path& packPath,
                           const std::filesystem::path& sourceFolder,
                           ShaderBackend shaderBackend) {
        m_packPath = packPath;
        bool packExists = std::filesystem::exists(packPath);

        if (!sourceFolder.empty() && std::filesystem::exists(sourceFolder)) {
            std::vector<AssetTOCEntry> knownEntries;
            if (packExists) {
                try { knownEntries = AssetPacker::readTOC(packPath); }
                catch (...) { packExists = false; }
            }

            auto newAssets = AssetCompiler::compileFolder(sourceFolder, knownEntries, shaderBackend);

            if (!packExists) {
                newAssets = AssetCompiler::compileFolder(sourceFolder, {}, shaderBackend);
                AssetPacker::packAll(packPath, newAssets);
                printf("[Assets] Built %zu assets -> %s\n",
                       newAssets.size(), packPath.string().c_str());
            } else if (!newAssets.empty()) {
                uint32_t n = AssetPacker::update(packPath, newAssets);
                printf("[Assets] Updated %u asset(s) in %s\n", n, packPath.string().c_str());
            } else {
                printf("[Assets] Pack up to date: %s\n", packPath.string().c_str());
            }
        } else if (!packExists) {
            throw std::runtime_error("[Assets] No pack found and no source folder: "
                                     + packPath.string());
        }

        auto toc = AssetPacker::readTOC(packPath);
        for (auto& e : toc) m_toc[e.id] = e;

        m_packFile.open(packPath, std::ios::binary);
        if (!m_packFile)
            throw std::runtime_error("[Assets] Cannot open pack: " + packPath.string());

        m_running = true;
        uint32_t nThreads = std::max(1u, std::thread::hardware_concurrency() / 2);
        for (uint32_t i = 0; i < nThreads; ++i)
            m_workers.emplace_back([this]{ workerLoop(); });

        printf("[Assets] Loaded TOC: %zu entries, %u worker threads\n",
               m_toc.size(), nThreads);
    }

    void AssetLoader::shutdown() {
        m_running = false;
        m_workCV.notify_all();
        for (auto& t : m_workers) if (t.joinable()) t.join();
        m_workers.clear();

        m_packFile.close();
        m_texCache.clear();
        m_audCache.clear();
        m_fontCache.clear();
        m_shaderCache.clear();
        m_binCache.clear();
        m_toc.clear();
    }

    std::vector<uint8_t> AssetLoader::readRawBlob(AssetID id) {
        auto it = m_toc.find(id);
        if (it == m_toc.end())
            throw std::runtime_error("[Assets] Asset not found in pack");
        auto& e = it->second;

        std::lock_guard lock(m_fileMutex);
        m_packFile.seekg(e.offset);
        std::vector<uint8_t> compressed(e.compressedSize);
        m_packFile.read((char*)compressed.data(), e.compressedSize);
        return decompressBlob(compressed, e.originalSize, e.compressMode, id.hash);
    }

    std::vector<uint8_t> AssetLoader::decompressBlob(const std::vector<uint8_t>& compressed,
                                                       uint32_t originalSize,
                                                       CompressMode mode, uint64_t seed) {
        if (mode == CompressMode::None) return compressed;

        auto tmp = compressed;
        uint8_t key = XOR_KEY ^ (uint8_t)(seed & 0xFF) ^ XOR_SALT;
        for (size_t i = 0; i < tmp.size(); ++i)
            tmp[i] ^= (key + (uint8_t)(i & 0xFF));

        std::vector<uint8_t> dst(originalSize);
        size_t r = ZSTD_decompress(dst.data(), originalSize, tmp.data(), tmp.size());
        if (ZSTD_isError(r))
            throw std::runtime_error(std::string("[Assets] zstd: ") + ZSTD_getErrorName(r));
        return dst;
    }

    std::shared_ptr<TextureAsset> AssetLoader::decodeTexture(const std::vector<uint8_t>& data,
                                                              AssetSubtype subtype) {
        auto t = std::make_shared<TextureAsset>();
        if (subtype == AssetSubtype::TexDDS) {
            t->isDDS  = true;
            t->pixels = data;
            return t;
        }
        if (data.size() < 12) return nullptr;
        auto r32 = [&](size_t o) {
            return (uint32_t)data[o] | ((uint32_t)data[o+1]<<8) |
                   ((uint32_t)data[o+2]<<16) | ((uint32_t)data[o+3]<<24);
        };
        t->width    = r32(0);
        t->height   = r32(4);
        t->channels = r32(8);
        t->pixels.assign(data.begin() + 12, data.end());
        return t;
    }

    std::shared_ptr<AudioAsset> AssetLoader::decodeAudio(const std::vector<uint8_t>& data,
                                                          AssetSubtype subtype) {
        auto a = std::make_shared<AudioAsset>();
        a->isCompressed = (subtype == AssetSubtype::AudioOGG ||
                           subtype == AssetSubtype::AudioMP3);
        a->data = data;
        return a;
    }

    std::shared_ptr<FontAsset> AssetLoader::decodeFont(const std::vector<uint8_t>& data) {
        auto f = std::make_shared<FontAsset>();
        f->ttfData = data;
        return f;
    }

    std::shared_ptr<ShaderAsset> AssetLoader::decodeShader(const std::vector<uint8_t>& data,
                                                             const AssetTOCEntry& e) {
        auto s = std::make_shared<ShaderAsset>();
        s->bytecode = data;
        s->stage    = e.subtype;
        s->backend  = e.shaderBackend;
        s->name     = std::filesystem::path(e.path).stem().string();
        return s;
    }

    std::shared_ptr<BinaryAsset> AssetLoader::decodeBinary(const std::vector<uint8_t>& data) {
        auto b = std::make_shared<BinaryAsset>();
        b->data = data;
        return b;
    }

    TextureHandle AssetLoader::loadTexture(std::string_view path) {
        AssetID id = makeAssetID(path);
        if (auto c = m_texCache.get(id)) return TextureHandle(c, id);

        auto it = m_toc.find(id);
        if (it == m_toc.end()) throw std::runtime_error("[Assets] Texture not found: " + std::string(path));

        auto data  = readRawBlob(id);
        auto asset = decodeTexture(data, it->second.subtype);
        m_texCache.insert(id, asset, asset->pixels.size());
        return TextureHandle(asset, id);
    }

    AudioHandle AssetLoader::loadAudio(std::string_view path) {
        AssetID id = makeAssetID(path);
        if (auto c = m_audCache.get(id)) return AudioHandle(c, id);

        auto it = m_toc.find(id);
        if (it == m_toc.end()) throw std::runtime_error("[Assets] Audio not found: " + std::string(path));

        auto data  = readRawBlob(id);
        auto asset = decodeAudio(data, it->second.subtype);
        m_audCache.insert(id, asset, asset->data.size());
        return AudioHandle(asset, id);
    }

    FontHandle AssetLoader::loadFont(std::string_view path) {
        AssetID id = makeAssetID(path);
        if (auto c = m_fontCache.get(id)) return FontHandle(c, id);

        auto data  = readRawBlob(id);
        auto asset = decodeFont(data);
        m_fontCache.insert(id, asset, asset->ttfData.size());
        return FontHandle(asset, id);
    }

    ShaderHandle AssetLoader::loadShader(std::string_view name, AssetSubtype stage) {
        AssetID id = makeShaderID(name, stage);
        if (auto c = m_shaderCache.get(id)) return ShaderHandle(c, id);

        auto it = m_toc.find(id);
        if (it == m_toc.end())
            throw std::runtime_error("[Assets] Shader not found: " + std::string(name));

        auto data  = readRawBlob(id);
        auto asset = decodeShader(data, it->second);
        m_shaderCache.insert(id, asset, asset->bytecode.size());
        return ShaderHandle(asset, id);
    }

    BinaryHandle AssetLoader::loadBinary(std::string_view path) {
        AssetID id = makeAssetID(path);
        if (auto c = m_binCache.get(id)) return BinaryHandle(c, id);

        auto data  = readRawBlob(id);
        auto asset = decodeBinary(data);
        m_binCache.insert(id, asset, asset->data.size());
        return BinaryHandle(asset, id);
    }

    void AssetLoader::loadTextureAsync(std::string_view path,
                                        std::function<void(TextureHandle)> cb) {
        AssetID id = makeAssetID(path);
        if (auto c = m_texCache.get(id)) { cb(TextureHandle(c, id)); return; }

        AsyncRequest req;
        req.id   = id;
        req.type = AssetType::Texture;
        req.onComplete = [this, cb, id](AssetID) {
            cb(TextureHandle(m_texCache.get(id), id));
        };
        { std::lock_guard lock(m_workMutex); m_workQueue.push(req); }
        m_workCV.notify_one();
    }

    void AssetLoader::loadAudioAsync(std::string_view path,
                                      std::function<void(AudioHandle)> cb) {
        AssetID id = makeAssetID(path);
        if (auto c = m_audCache.get(id)) { cb(AudioHandle(c, id)); return; }

        AsyncRequest req;
        req.id   = id;
        req.type = AssetType::Audio;
        req.onComplete = [this, cb, id](AssetID) {
            cb(AudioHandle(m_audCache.get(id), id));
        };
        { std::lock_guard lock(m_workMutex); m_workQueue.push(req); }
        m_workCV.notify_one();
    }

    void AssetLoader::tick() {
        std::vector<PendingCB> cbs;
        { std::lock_guard lock(m_cbMutex); cbs.swap(m_pendingCBs); }
        for (auto& p : cbs) if (p.cb) p.cb(p.id);
    }

    bool AssetLoader::isReady(AssetID id) const {
        return m_texCache.contains(id) || m_audCache.contains(id) ||
               m_fontCache.contains(id) || m_shaderCache.contains(id) ||
               m_binCache.contains(id);
    }

    void AssetLoader::workerLoop() {
        while (m_running) {
            AsyncRequest req;
            {
                std::unique_lock lock(m_workMutex);
                m_workCV.wait(lock, [this]{ return !m_workQueue.empty() || !m_running; });
                if (!m_running && m_workQueue.empty()) return;
                req = m_workQueue.front();
                m_workQueue.pop();
            }

            try {
                auto it = m_toc.find(req.id);
                if (it == m_toc.end()) continue;

                auto data = readRawBlob(req.id);

                switch (req.type) {
                    case AssetType::Texture: {
                        auto a = decodeTexture(data, it->second.subtype);
                        m_texCache.insert(req.id, a, a->pixels.size());
                        break;
                    }
                    case AssetType::Audio: {
                        auto a = decodeAudio(data, it->second.subtype);
                        m_audCache.insert(req.id, a, a->data.size());
                        break;
                    }
                    default: break;
                }

                if (req.onComplete) {
                    std::lock_guard lock(m_cbMutex);
                    m_pendingCBs.push_back({ req.id, req.onComplete });
                }
            } catch (const std::exception& e) {
                fprintf(stderr, "[Assets] Async load failed: %s\n", e.what());
            }
        }
    }
}