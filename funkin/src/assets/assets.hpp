// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "assets_types.hpp"
#include "shared/log.hpp"
#include "app/projects/project.hpp"
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <filesystem>
#include "loaders/texture_loader.hpp"

namespace Funkin::Assets {
    class AssetManager {
    public:
        static AssetManager& get();

        void init();
        void shutdown();
        void update();

        template<typename T> AssetHandle<T> getFallback();
        template<typename T> std::shared_ptr<T> loadFromDisk(const std::string& id, const std::string& group);
        template<typename T> std::shared_ptr<T> loadFromDiskRaw(const std::filesystem::path& path, const std::string& id, const std::string& group);

        template<typename T>
        AssetHandle<T> load(const std::string& id, const std::string& group = "") {
            auto it = m_cache.find(id);
            if (it != m_cache.end()) {
                auto ptr = std::dynamic_pointer_cast<T>(it->second);
                if (ptr) return AssetHandle<T>(ptr);
            }

            auto ptr = loadFromDisk<T>(id, group);
            if (!ptr) {
                LOG_WARN("Asset not found: {} — using fallback", id);
                return getFallback<T>();
            }

            m_cache[id] = ptr;
            if (!group.empty()) m_groups[group].push_back(id);
            LOG_PRINT("Loaded: {} ({} bytes)", id, ptr->sizeBytes);
            return AssetHandle<T>(ptr);
        }

        template<typename T>
        void loadAsync(const std::string& id, const std::string& group,
                       std::function<void(AssetHandle<T>)> callback) {
            auto it = m_cache.find(id);
            if (it != m_cache.end()) {
                auto ptr = std::dynamic_pointer_cast<T>(it->second);
                if (ptr) { callback(AssetHandle<T>(ptr)); return; }
            }

            auto path = resolvePath(id);
            if (path.empty()) {
                LOG_WARN("Asset not found: {}", id);
                callback(getFallback<T>());
                return;
            }

            std::thread([this, path, id, group, callback]() {
                auto handle = loadFromDiskRaw<T>(path, id, group);
                std::lock_guard<std::mutex> lock(m_completionMutex);
                m_completions.push([this, id, group, handle, callback]() {
                    if (!handle) {
                        callback(getFallback<T>());
                        return;
                    }
                    m_cache[id] = handle;
                    if (!group.empty()) m_groups[group].push_back(id);
                    LOG_PRINT("Async loaded: {} ({} bytes)", id, handle->sizeBytes);
                    callback(AssetHandle<T>(handle));
                });
            }).detach();
        }

        void unloadGroup(const std::string& group);
        void unloadAll();

        size_t        cachedCount()      const { return m_cache.size(); }
        uint64_t      totalBytesLoaded() const;
        TextureHandle missingTexture()   const { return m_missingTexture; }

    private:
        struct PendingUpload {
            Loaders::PendingTextureUpload                    data;
            std::function<void(std::shared_ptr<Texture>)>   onComplete;
        };

        AssetManager() : m_missingTexture(nullptr) {}

        std::filesystem::path resolvePath(const std::string& id);

        std::unordered_map<std::string, std::shared_ptr<AssetBase>> m_cache;
        std::unordered_map<std::string, std::vector<std::string>>   m_groups;

        std::mutex                        m_completionMutex;
        std::queue<std::function<void()>> m_completions;

        std::mutex                        m_uploadMutex;
        std::queue<PendingUpload>         m_pendingUploads;

        TextureHandle m_missingTexture;
    };

    // this is fucking awful
    template<> AssetHandle<Texture> AssetManager::getFallback<Texture>();
    template<> AssetHandle<AudioClip> AssetManager::getFallback<AudioClip>();
    template<> AssetHandle<JsonAsset> AssetManager::getFallback<JsonAsset>();
    template<> AssetHandle<Font> AssetManager::getFallback<Font>();
    template<> AssetHandle<SparrowAtlas> AssetManager::getFallback<SparrowAtlas>();

    template<> std::shared_ptr<Texture> AssetManager::loadFromDisk<Texture>(const std::string& id, const std::string& group);
    template<> std::shared_ptr<AudioClip> AssetManager::loadFromDisk<AudioClip>(const std::string& id, const std::string& group);
    template<> std::shared_ptr<JsonAsset> AssetManager::loadFromDisk<JsonAsset>(const std::string& id, const std::string& group);
    template<> std::shared_ptr<Font> AssetManager::loadFromDisk<Font>(const std::string& id, const std::string& group);
    template<> std::shared_ptr<SparrowAtlas> AssetManager::loadFromDisk<SparrowAtlas>(const std::string& id, const std::string& group);

    template<> std::shared_ptr<Texture> AssetManager::loadFromDiskRaw<Texture>(const std::filesystem::path& path, const std::string& id, const std::string& group);
    template<> std::shared_ptr<AudioClip> AssetManager::loadFromDiskRaw<AudioClip>(const std::filesystem::path& path, const std::string& id, const std::string& group);
    template<> std::shared_ptr<JsonAsset> AssetManager::loadFromDiskRaw<JsonAsset>(const std::filesystem::path& path, const std::string& id, const std::string& group);
    template<> std::shared_ptr<Font> AssetManager::loadFromDiskRaw<Font>(const std::filesystem::path& path, const std::string& id, const std::string& group);
    template<> std::shared_ptr<SparrowAtlas> AssetManager::loadFromDiskRaw<SparrowAtlas>(const std::filesystem::path& path, const std::string& id, const std::string& group);

    template<>
    inline void AssetManager::loadAsync<Texture>(const std::string& id, const std::string& group,
                                                 std::function<void(AssetHandle<Texture>)> callback) {
        auto it = m_cache.find(id);
        if (it != m_cache.end()) {
            auto ptr = std::dynamic_pointer_cast<Texture>(it->second);
            if (ptr) { callback(AssetHandle<Texture>(ptr)); return; }
        }

        auto path = resolvePath(id);
        if (path.empty()) {
            LOG_WARN("Asset not found: {}", id);
            callback(getFallback<Texture>());
            return;
        }

        std::thread([this, path, id, group, callback]() {
            auto pending = Loaders::prepareTexture(path, id, group);
            if (!pending) {
                std::lock_guard<std::mutex> lock(m_completionMutex);
                m_completions.push([this, callback]() {
                    callback(getFallback<Texture>());
                });
                return;
            }

            std::lock_guard<std::mutex> lock(m_uploadMutex);
            m_pendingUploads.push({
                std::move(*pending),
                [this, id, group, callback](std::shared_ptr<Texture> tex) {
                    if (tex) {
                        m_cache[id] = tex;
                        if (!group.empty()) m_groups[group].push_back(id);
                        LOG_PRINT("Async texture ready: {} ({} bytes)", id, tex->sizeBytes);
                        callback(AssetHandle<Texture>(tex));
                    } else {
                        callback(getFallback<Texture>());
                    }
                }
            });
        }).detach();
    }

    template<typename T>
    AssetHandle<T> Load(const std::string& id, const std::string& group = "") {
        return AssetManager::get().load<T>(id, group);
    }

    template<typename T>
    void LoadAsync(const std::string& id, const std::string& group,
                   std::function<void(AssetHandle<T>)> cb) {
        AssetManager::get().loadAsync<T>(id, group, cb);
    }
}