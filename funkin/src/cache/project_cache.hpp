// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <functional>
#include <atomic>
#include <mutex>
#include <string>
#include <nlohmann/json.hpp>
#include "shared/log.hpp"

namespace Funkin::Cache {
    struct CacheWarmProgress {
        std::atomic<int>  completed { 0 };
        std::atomic<int>  total     { 0 };
        std::atomic<bool> done      { false };
        std::atomic<bool> failed    { false };

        std::string currentFile;
        std::mutex  mutex;

        float percent() const {
            int t = total.load();
            return t > 0 ? (completed.load() / (float)t) * 100.f : 0.f;
        }
    };

    class ProjectCache {
    public:
        static ProjectCache& get();

        void warmAsync(
            const std::filesystem::path& assetsRoot,
            const std::filesystem::path& cacheDir,
            std::function<void()> onComplete = nullptr
        );

        std::filesystem::path getCachedPath(const std::filesystem::path& srcPath) const;
        std::filesystem::path getThumbPath(const std::filesystem::path& srcPath) const;

        CacheWarmProgress& progress() { return m_progress; }

    private:
        ProjectCache() = default;

        void warmWorker(
            const std::filesystem::path& assetsRoot,
            const std::filesystem::path& cacheDir,
            std::function<void()> onComplete
        );

        void writeManifest(
            const std::filesystem::path& cacheDir,
            const nlohmann::json& manifest
        );

        uint64_t hashFile(const std::filesystem::path& path) const;

        CacheWarmProgress m_progress;
    };
}