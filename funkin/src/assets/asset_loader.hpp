// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"
#include "asset_handle.hpp"
#include "asset_cache.hpp"
#include "asset_packer.hpp"

namespace Funkin::Assets {
    struct AsyncRequest {
        AssetID   id;
        AssetType type;
        std::function<void(AssetID)> onComplete;
    };

    class AssetLoader {
    public:
        static AssetLoader& get();

        void init(const std::filesystem::path& packPath,
                  const std::filesystem::path& sourceFolder = "",
                  ShaderBackend shaderBackend = ShaderBackend::DX12);
        void shutdown();

        TextureHandle loadTexture(std::string_view path);
        AudioHandle   loadAudio(std::string_view path);
        FontHandle    loadFont(std::string_view path);
        BinaryHandle  loadBinary(std::string_view path);
        ShaderHandle  loadShader(std::string_view name, AssetSubtype stage);

        void loadTextureAsync(std::string_view path, std::function<void(TextureHandle)> cb);
        void loadAudioAsync(std::string_view path,   std::function<void(AudioHandle)>   cb);
        void tick();

        bool isReady(AssetID id) const;

        size_t textureCacheUsed()  const { return m_texCache.usedBytes(); }
        size_t audioCacheUsed()    const { return m_audCache.usedBytes(); }
        size_t shaderCacheCount()  const { return m_shaderCache.count(); }

    private:
        AssetLoader() = default;

        std::vector<uint8_t> readRawBlob(AssetID id);
        std::vector<uint8_t> decompressBlob(const std::vector<uint8_t>& compressed,
                                             uint32_t originalSize,
                                             CompressMode mode, uint64_t seed);

        std::shared_ptr<TextureAsset> decodeTexture(const std::vector<uint8_t>& data,
                                                     AssetSubtype subtype);
        std::shared_ptr<AudioAsset>   decodeAudio(const std::vector<uint8_t>& data,
                                                   AssetSubtype subtype);
        std::shared_ptr<FontAsset>    decodeFont(const std::vector<uint8_t>& data);
        std::shared_ptr<ShaderAsset>  decodeShader(const std::vector<uint8_t>& data,
                                                    const AssetTOCEntry& e);
        std::shared_ptr<BinaryAsset>  decodeBinary(const std::vector<uint8_t>& data);

        void workerLoop();

        std::filesystem::path                                  m_packPath;
        std::ifstream                                          m_packFile;
        std::mutex                                             m_fileMutex;
        std::unordered_map<AssetID, AssetTOCEntry, AssetIDHasher> m_toc;

        mutable AssetCache<TextureAsset> m_texCache   { 256 * 1024 * 1024 }; // 256 MB
        mutable AssetCache<AudioAsset>   m_audCache   { 128 * 1024 * 1024 }; // 128 MB
        mutable AssetCache<FontAsset>    m_fontCache  {  32 * 1024 * 1024 }; //  32 MB
        mutable AssetCache<ShaderAsset>  m_shaderCache{  64 * 1024 * 1024 }; //  64 MB
        mutable AssetCache<BinaryAsset>  m_binCache   {  64 * 1024 * 1024 }; //  64 MB

        std::vector<std::thread>  m_workers;
        std::queue<AsyncRequest>  m_workQueue;
        std::mutex                m_workMutex;
        std::condition_variable   m_workCV;
        std::atomic<bool>         m_running{ false };

        struct PendingCB {
            AssetID id;
            std::function<void(AssetID)> cb;
        };
        std::vector<PendingCB> m_pendingCBs;
        std::mutex             m_cbMutex;
    };
}