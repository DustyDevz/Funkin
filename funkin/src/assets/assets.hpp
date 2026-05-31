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

namespace Funkin::Assets {
    class AssetManager {
    public:
        static AssetManager& get();

        void init();
        void shutdown();
        void update();

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
            std::thread([this, id, group, callback]() {
                auto handle = load<T>(id, group);
                std::lock_guard<std::mutex> lock(m_completionMutex);
                m_completions.push([callback, handle]() { callback(handle); });
            }).detach();
        }

        void unloadGroup(const std::string& group);
        void unloadAll();

        size_t   cachedCount()     const { return m_cache.size(); }
        uint64_t totalBytesLoaded() const;
        TextureHandle missingTexture() const { return m_missingTexture; }

    private:
        AssetManager() : m_missingTexture(nullptr) {}

        std::filesystem::path resolvePath(const std::string& id);

        template<typename T>
        std::shared_ptr<T> loadFromDisk(const std::string& id, const std::string& group);

        template<typename T>
        AssetHandle<T> getFallback();

        std::unordered_map<std::string, std::shared_ptr<AssetBase>> m_cache;
        std::unordered_map<std::string, std::vector<std::string>> m_groups;
        std::mutex                                m_completionMutex;
        std::queue<std::function<void()>> m_completions;

        TextureHandle m_missingTexture;
    };

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