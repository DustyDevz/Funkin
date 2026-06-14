// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "texture_loader.hpp"
#include "cache/project_cache.hpp"
#include <fstream>
#include <vector>

#include <bimg/bimg.h>
#include <bimg/encode.h>
#include "filesystem/filesystem.hpp"
#include "cache/cache.hpp"

#include <bx/allocator.h>
#include <bx/file.h>
#include <bimg/bimg.h>
#include <bimg/encode.h>
#include <bgfx/bgfx.h>
#include <lz4.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Funkin::Assets::Loaders {
    static bgfx::TextureHandle uploadRGBA(const uint8_t* pixels, uint32_t w, uint32_t h) {
        const bgfx::Memory* mem = bgfx::copy(pixels, w * h * 4);
        return bgfx::createTexture2D(
            (uint16_t)w, (uint16_t)h,
            false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            mem
        );
    }

    static std::filesystem::path getTextureCachePath(const std::filesystem::path& srcPath) {
        std::ifstream f(srcPath, std::ios::binary);
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), {});
        uint64_t hash = XXH3_64bits(bytes.data(), bytes.size());

        auto cacheDir = Funkin::Filesystem::resolve("cache://textures");
        std::filesystem::create_directories(cacheDir);

        return std::filesystem::path(
            cacheDir / std::format("{:x}.ktx", hash)).make_preferred();
    }

    std::optional<PendingTextureUpload> prepareTextureFromMemory(
        const std::vector<uint8_t>& bytes,
        const std::string& id,
        const std::string& group) {

        int w, h, channels;
        uint8_t* pixels = stbi_load_from_memory(bytes.data(), (int)bytes.size(), &w, &h, &channels, 4);
        if (!pixels) {
            LOG_WARN("stb_image memory load failed for: {}", id);
            return std::nullopt;
        }

        LOG_PRINT("Memory PNG decoded: {}x{}", w, h);

        std::vector<uint8_t> raw(pixels, pixels + w * h * 4);
        stbi_image_free(pixels);

        return PendingTextureUpload{ id, group, std::move(raw), (uint32_t)w, (uint32_t)h, false };
    }

    std::optional<PendingTextureUpload> prepareTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        LOG_PRINT("prepareTexture: {}", path.string());

        int w, h, channels;
        uint8_t* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
        if (!pixels) {
            LOG_WARN("stb_image failed: {} — {}", path.string(), stbi_failure_reason());
            return std::nullopt;
        }

        LOG_PRINT("PNG decoded: {}x{}", w, h);

        std::vector<uint8_t> raw(pixels, pixels + w * h * 4);
        stbi_image_free(pixels);

        return PendingTextureUpload{ id, group, std::move(raw), (uint32_t)w, (uint32_t)h, false };
    }

    std::shared_ptr<Texture> uploadPendingTexture(PendingTextureUpload& pending) {
        if (!pending.isKtx) {
            auto handle = uploadRGBA(pending.bytes.data(), pending.width, pending.height);
            if (!bgfx::isValid(handle)) {
                LOG_ERR("bgfx RGBA upload failed: {}", pending.id);
                return nullptr;
            }
            auto asset       = std::make_shared<Texture>();
            asset->id        = pending.id;
            asset->group     = pending.group;
            asset->width     = pending.width;
            asset->height    = pending.height;
            asset->sizeBytes = pending.bytes.size();
            asset->handle    = handle;
            asset->loaded    = true;
            return asset;
        }

        const bgfx::Memory* mem = bgfx::copy(
            pending.bytes.data(), (uint32_t)pending.bytes.size());
        bgfx::TextureInfo info{};
        auto handle = bgfx::createTexture(mem,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, 0, &info);

        if (!bgfx::isValid(handle)) {
            LOG_ERR("bgfx KTX upload failed: {}", pending.id);
            return nullptr;
        }

        auto asset       = std::make_shared<Texture>();
        asset->id        = pending.id;
        asset->group     = pending.group;
        asset->width     = info.width;
        asset->height    = info.height;
        asset->sizeBytes = pending.bytes.size();
        asset->handle    = handle;
        asset->loaded    = true;
        return asset;
    }

    std::shared_ptr<Texture> loadTexture(
        const std::string& virtualPath,
        const std::string& id,
        const std::string& group) {

        if (Funkin::Filesystem::isEngineResource(virtualPath)) {
            auto bytes = Funkin::Filesystem::readBytes(virtualPath);
            if (bytes.empty()) {
                LOG_ERR("Failed to read engine resource: {}", virtualPath);
                return nullptr;
            }
            auto pending = prepareTextureFromMemory(bytes, id, group);
            if (!pending) return nullptr;
            return uploadPendingTexture(*pending);
        }

        std::filesystem::path path(virtualPath);
        auto ext = path.extension().string();
        for (auto& c : ext) c = (char)tolower(c);

        if (ext == ".dds" || ext == ".ktx") {
            std::ifstream f(path, std::ios::binary);
            if (!f) return nullptr;
            std::vector<uint8_t> bytes(
                (std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>());
            PendingTextureUpload pending{ id, group, std::move(bytes), 0, 0, true };
            return uploadPendingTexture(pending);
        }

        auto cachedPath = Funkin::Cache::ProjectCache::get().getCachedPath(path);
        if (!cachedPath.empty()) {
            std::ifstream f(cachedPath, std::ios::binary);
            if (f) {
                uint8_t magic[4]; uint8_t version; int w, h;
                f.read(reinterpret_cast<char*>(magic),   4);
                f.read(reinterpret_cast<char*>(&version),1);
                f.read(reinterpret_cast<char*>(&w),      4);
                f.read(reinterpret_cast<char*>(&h),      4);

               if (magic[0]=='F' && magic[1]=='K' && magic[2]=='T' && magic[3]=='X' && w > 0 && h > 0) {
                int rawSize, compressedSize;
                f.read(reinterpret_cast<char*>(&rawSize),        4);
                f.read(reinterpret_cast<char*>(&compressedSize), 4);

                std::vector<uint8_t> compressed(compressedSize);
                f.read(reinterpret_cast<char*>(compressed.data()), compressedSize);

                if (f) {
                    std::vector<uint8_t> pixels(rawSize);
                    int result = LZ4_decompress_safe(
                        reinterpret_cast<const char*>(compressed.data()),
                        reinterpret_cast<char*>(pixels.data()),
                        compressedSize,
                        rawSize
                    );

                    if (result == rawSize) {
                        LOG_PRINT("cache hit {}", path.filename().string());
                        PendingTextureUpload pending{ id, group, std::move(pixels), (uint32_t)w, (uint32_t)h, false };
                        return uploadPendingTexture(pending);
                    }
                    LOG_WARN("lz4 decompress failed for {}", path.filename().string());
                }
            }
                LOG_WARN("cache file corrupt, falling back: {}", cachedPath.string());
            }
        }

        LOG_PRINT("cache miss, decoding {}", path.filename().string());
        auto pending = prepareTexture(path, id, group);
        if (!pending) return nullptr;
        return uploadPendingTexture(*pending);
    }

    std::shared_ptr<Texture> loadTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {
        return loadTexture(path.string(), id, group);
    }

    TextureHandle createMissingTexture() {
        // 16x16 pink/black checkerboard
        // half life :o
        constexpr uint32_t SIZE  = 16;
        constexpr uint32_t PINK  = (0xFF << 0) | (0x69 << 8) | (0xB4 << 16) | (0xFF << 24);
        constexpr uint32_t BLACK = (0x00 << 0) | (0x00 << 8) | (0x00 << 16) | (0xFF << 24);

        uint32_t pixels[SIZE * SIZE];
        for (uint32_t y = 0; y < SIZE; y++)
            for (uint32_t x = 0; x < SIZE; x++)
                pixels[y * SIZE + x] = (((x / 4) + (y / 4)) % 2 == 0) ? PINK : BLACK;

        auto asset       = std::make_shared<Texture>();
        asset->id        = "_missing";
        asset->group     = "";
        asset->width     = SIZE;
        asset->height    = SIZE;
        asset->sizeBytes = sizeof(pixels);
        asset->handle    = uploadRGBA(reinterpret_cast<uint8_t*>(pixels), SIZE, SIZE);
        asset->loaded    = bgfx::isValid(asset->handle);

        if (!asset->loaded)
            LOG_ERR("Failed to create missing texture");
        else
            LOG_PRINT("Missing texture created ({}x{})", SIZE, SIZE);

        return TextureHandle(asset);
    }
}