// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "cache.hpp"
#include "filesystem/filesystem.hpp"

namespace Funkin::Cache {
    static std::filesystem::path metaPath() {
        return Funkin::Filesystem::resolve("cache://cache_meta.json");
    }

    struct CacheEntry {
        std::string cachePath;
        std::string srcPath;
        uint64_t    lastAccess;
        uint64_t    sizeBytes;
    };

    static std::vector<CacheEntry> s_entries;
    static bool s_dirty = false;

    static void loadMeta() {
        std::ifstream f(metaPath());
        if (!f) return;

        try {
            auto j = nlohmann::json::parse(f);
            for (auto& e : j) {
                s_entries.push_back({
                    e["cachePath"].get<std::string>(),
                    e["srcPath"].get<std::string>(),
                    e["lastAccess"].get<uint64_t>(),
                    e["sizeBytes"].get<uint64_t>()
                });
            }
        } catch (...) {
            LOG_WARN("Cache meta parse failed, starting fresh");
            s_entries.clear();
        }
    }

    static void saveMeta() {
        if (!s_dirty) return;
        nlohmann::json j = nlohmann::json::array();
        for (auto& e : s_entries) {
            j.push_back({
                { "cachePath",   e.cachePath   },
                { "srcPath",     e.srcPath     },
                { "lastAccess",  e.lastAccess  },
                { "sizeBytes",   e.sizeBytes   }
            });
        }
        std::ofstream f(metaPath());
        f << j.dump(2);
        s_dirty = false;
    }

    void init() {
        loadMeta();
        removeOrphans();
        evictIfNeeded();
    }

    void trackAccess(const std::filesystem::path& cachePath) {
        auto key = cachePath.string();
        auto it  = std::find_if(s_entries.begin(), s_entries.end(),
            [&](const CacheEntry& e) { return e.cachePath == key; });

        uint64_t now = (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (it != s_entries.end()) {
            it->lastAccess = now;
        } else {
            uint64_t size = std::filesystem::exists(cachePath)
                ? std::filesystem::file_size(cachePath) : 0;
            s_entries.push_back({ key, "", now, size });
        }

        s_dirty = true;
        saveMeta();
    }

    void trackSource(const std::filesystem::path& cachePath, const std::filesystem::path& srcPath) {
        auto key = cachePath.string();
        auto it  = std::find_if(s_entries.begin(), s_entries.end(),
            [&](const CacheEntry& e) { return e.cachePath == key; });

        if (it != s_entries.end()) {
            it->srcPath = srcPath.string();
        } else {
            uint64_t size = std::filesystem::exists(cachePath)
                ? std::filesystem::file_size(cachePath) : 0;
            s_entries.push_back({ key, srcPath.string(), 0, size });
        }

        s_dirty = true;
        saveMeta();
    }

    void removeOrphans() {
        bool changed = false;
        s_entries.erase(std::remove_if(s_entries.begin(), s_entries.end(),
            [&](const CacheEntry& e) {
                bool cacheGone  = !std::filesystem::exists(e.cachePath);
                bool sourceGone = !e.srcPath.empty() && !std::filesystem::exists(e.srcPath);

                if (cacheGone || sourceGone) {
                    if (std::filesystem::exists(e.cachePath))
                        std::filesystem::remove(e.cachePath);
                    LOG_PRINT("Cache orphan removed: {}", e.cachePath);
                    changed = true;
                    return true;
                }
                return false;
            }), s_entries.end());

        if (changed) { s_dirty = true; saveMeta(); }
    }

    uint64_t totalSizeBytes() {
        uint64_t total = 0;
        for (auto& e : s_entries) total += e.sizeBytes;
        return total;
    }

    void evictIfNeeded() {
        if (totalSizeBytes() <= MAX_CACHE_BYTES) return;

        std::sort(s_entries.begin(), s_entries.end(),
            [](const CacheEntry& a, const CacheEntry& b) {
                return a.lastAccess < b.lastAccess;
            });

        while (totalSizeBytes() > MAX_CACHE_BYTES && !s_entries.empty()) {
            auto& oldest = s_entries.front();
            LOG_PRINT("Cache evicting LRU: {}", oldest.cachePath);
            if (std::filesystem::exists(oldest.cachePath))
                std::filesystem::remove(oldest.cachePath);
            s_entries.erase(s_entries.begin());
            s_dirty = true;
        }

        saveMeta();
    }

    void clearAll() {
        for (auto& e : s_entries) {
            if (std::filesystem::exists(e.cachePath))
                std::filesystem::remove(e.cachePath);
        }
        s_entries.clear();
        s_dirty = true;
        saveMeta();
        LOG_PRINT("Cache cleared");
    }
}