// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <cstdint>

namespace Funkin::Cache {
    constexpr uint64_t MAX_CACHE_BYTES = 512ULL * 1024 * 1024; // 512MB

    void init();
    void evictIfNeeded();
    void clearAll();
    uint64_t totalSizeBytes();

    void trackAccess(const std::filesystem::path& cachePath);
    void trackSource(const std::filesystem::path& cachePath, const std::filesystem::path& srcPath);
    void removeOrphans();
}