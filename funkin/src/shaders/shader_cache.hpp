// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <filesystem>

namespace Funkin::Shader::Cache {
    std::filesystem::path resolveCachePath(const std::string& name, const std::string& stage, const std::string& suffix);
    bgfx::ShaderHandle    loadFromCache(const std::filesystem::path& path);
    bool                  isCached(const std::filesystem::path& path);
    void                  ensureCacheDir();
}