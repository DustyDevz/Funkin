// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "assets.hpp"
#include "loaders/texture_loader.hpp"
#include "loaders/audio_loader.hpp"
#include "loaders/json_loader.hpp"
#include "loaders/font_loader.hpp"

namespace Funkin::Assets {
    AssetManager& AssetManager::get() {
        static AssetManager s;
        return s;
    }

    void AssetManager::init() {
        LOG_PRINT("AssetManager init");
        m_missingTexture = Loaders::createMissingTexture();
    }

    void AssetManager::shutdown() {
        unloadAll();
        LOG_PRINT("AssetManager shutdown");
    }

    void AssetManager::update() {
        std::lock_guard<std::mutex> lock(m_completionMutex);
        while (!m_completions.empty()) {
            m_completions.front()();
            m_completions.pop();
        }
    }

    std::filesystem::path AssetManager::resolvePath(const std::string& id) {
        auto loose = Funkin::App::Project::get().assets / id;
        if (std::filesystem::exists(loose)) return loose;
        return {};
    }

    void AssetManager::unloadGroup(const std::string& group) {
        auto it = m_groups.find(group);
        if (it == m_groups.end()) return;

        for (auto& id : it->second) {
            auto cit = m_cache.find(id);
            if (cit != m_cache.end()) {
                if (cit->second.use_count() == 1) {
                    LOG_PRINT("Unloading: {}", id);
                    m_cache.erase(cit);
                }
            }
        }
        m_groups.erase(it);
    }

    void AssetManager::unloadAll() {
        m_cache.clear();
        m_groups.clear();
    }

    uint64_t AssetManager::totalBytesLoaded() const {
        uint64_t total = 0;
        for (auto& [id, asset] : m_cache)
            total += asset->sizeBytes;
        return total;
    }

    template<>
    std::shared_ptr<Texture> AssetManager::loadFromDisk<Texture>(
        const std::string& id, const std::string& group) {
        auto path = resolvePath(id);
        if (path.empty()) return nullptr;
        return Loaders::loadTexture(path, id, group);
    }

    template<>
    std::shared_ptr<AudioClip> AssetManager::loadFromDisk<AudioClip>(
        const std::string& id, const std::string& group) {
        auto path = resolvePath(id);
        if (path.empty()) return nullptr;
        return Loaders::loadAudio(path, id, group);
    }

    template<>
    std::shared_ptr<JsonAsset> AssetManager::loadFromDisk<JsonAsset>(
        const std::string& id, const std::string& group) {
        auto path = resolvePath(id);
        if (path.empty()) return nullptr;
        return Loaders::loadJson(path, id, group);
    }

    template<>
    std::shared_ptr<Font> AssetManager::loadFromDisk<Font>(
        const std::string& id, const std::string& group) {
        auto path = resolvePath(id);
        if (path.empty()) return nullptr;
        return Loaders::loadFont(path, id, group);
    }

    template<>
    AssetHandle<Texture> AssetManager::getFallback<Texture>() {
        return m_missingTexture;
    }

    template<>
    AssetHandle<AudioClip> AssetManager::getFallback<AudioClip>() {
        return NullHandle<AudioClip>();
    }

    template<>
    AssetHandle<JsonAsset> AssetManager::getFallback<JsonAsset>() {
        return NullHandle<JsonAsset>();
    }

    template<>
    AssetHandle<Font> AssetManager::getFallback<Font>() {
        return NullHandle<Font>();
    }
}