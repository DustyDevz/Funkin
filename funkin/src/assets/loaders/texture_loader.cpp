// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "texture_loader.hpp"
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
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

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

        auto pending = prepareTexture(path, id, group);
        if (!pending) return nullptr;
        return uploadPendingTexture(*pending);
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