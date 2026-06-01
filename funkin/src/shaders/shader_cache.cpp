// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "shader_cache.hpp"
#include "filesystem/filesystem.hpp"

namespace Funkin::Shader::Cache {
    void ensureCacheDir() {
        std::filesystem::create_directories(
            Funkin::Filesystem::resolve("cache://shaders"));
    }

    std::filesystem::path resolveCachePath(const std::string& name, const std::string& stage, const std::string& suffix, uint64_t hash) {
        return std::filesystem::path(Funkin::Filesystem::resolve(
            std::format("cache://shaders/{}_{}_{:x}_{}.bin", name, stage, hash, suffix)
        )).make_preferred();
    }

    bool isCached(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::file_size(path) > 0;
    }

    bgfx::ShaderHandle loadFromCache(const std::filesystem::path& path) {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f) return BGFX_INVALID_HANDLE;

        std::streamsize size = f.tellg();
        f.seekg(0, std::ios::beg);

        std::vector<uint8_t> bytes(size);
        if (!f.read(reinterpret_cast<char*>(bytes.data()), size))
            return BGFX_INVALID_HANDLE;

        if (bytes.empty()) return BGFX_INVALID_HANDLE;

        const bgfx::Memory* mem = bgfx::copy(bytes.data(), (uint32_t)bytes.size());
        return bgfx::createShader(mem);
    }
}