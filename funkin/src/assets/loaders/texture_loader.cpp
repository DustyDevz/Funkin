// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "texture_loader.hpp"
#include <fstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Funkin::Assets::Loaders {
    static bgfx::TextureHandle uploadRGBA(
        const uint8_t* pixels, uint32_t w, uint32_t h) {
        const bgfx::Memory* mem = bgfx::copy(pixels, w * h * 4);
        return bgfx::createTexture2D(
            (uint16_t)w, (uint16_t)h,
            false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            mem
        );
    }

    static std::shared_ptr<Texture> loadSTB(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        int w, h, channels;
        uint8_t* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
        if (!pixels) {
            LOG_WARN("stb_image failed: {} — {}", path.string(), stbi_failure_reason());
            return nullptr;
        }

        auto asset          = std::make_shared<Texture>();
        asset->id           = id;
        asset->group        = group;
        asset->width        = (uint32_t)w;
        asset->height       = (uint32_t)h;
        asset->sizeBytes    = (uint64_t)(w * h * 4);
        asset->handle       = uploadRGBA(pixels, w, h);
        asset->loaded       = bgfx::isValid(asset->handle);

        stbi_image_free(pixels);

        if (!asset->loaded) {
            LOG_ERR("bgfx texture upload failed: {}", id);
            return nullptr;
        }
        return asset;
    }

    static std::shared_ptr<Texture> loadDDS(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        std::ifstream f(path, std::ios::binary);
        if (!f) return nullptr;

        std::vector<uint8_t> bytes(
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()
        );

        const bgfx::Memory* mem = bgfx::copy(bytes.data(), (uint32_t)bytes.size());
        bgfx::TextureHandle handle = bgfx::createTexture(mem,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, 0, nullptr);

        if (!bgfx::isValid(handle)) {
            LOG_ERR("DDS load failed: {}", id);
            return nullptr;
        }

        bgfx::TextureInfo info{};
        bgfx::calcTextureSize(info, 0, 0, 0, false, false, 1, bgfx::TextureFormat::Unknown);

        auto asset       = std::make_shared<Texture>();
        asset->id        = id;
        asset->group     = group;
        asset->handle    = handle;
        asset->sizeBytes = bytes.size();
        asset->loaded    = true;
        return asset;
    }

    std::shared_ptr<Texture> loadTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        auto ext = path.extension().string();
        for (auto& c : ext) c = (char)tolower(c);

        if (ext == ".dds" || ext == ".ktx")
            return loadDDS(path, id, group);

        // png, jpg, jpeg, bmp, tga, gif, psd, hdr, pic, qoi
        // this should be enough, RIGHT?!
        return loadSTB(path, id, group);
    }

    TextureHandle createMissingTexture() {
        // 16x16 pink/black checkerboard
        // half life :o
        constexpr uint32_t SIZE = 16;
        constexpr uint32_t PINK  = 0xFF69B4FF;
        constexpr uint32_t BLACK = 0x000000FF;

        uint32_t pixels[SIZE * SIZE];
        for (uint32_t y = 0; y < SIZE; y++) {
            for (uint32_t x = 0; x < SIZE; x++) {
                bool checker = ((x / 4) + (y / 4)) % 2 == 0;
                pixels[y * SIZE + x] = checker ? PINK : BLACK;
            }
        }

        auto asset       = std::make_shared<Texture>();
        asset->id        = "_missing";
        asset->group     = "";
        asset->width     = SIZE;
        asset->height    = SIZE;
        asset->sizeBytes = sizeof(pixels);
        asset->handle    = uploadRGBA(
            reinterpret_cast<uint8_t*>(pixels), SIZE, SIZE);
        asset->loaded    = bgfx::isValid(asset->handle);

        if (!asset->loaded)
            LOG_ERR("Failed to create missing texture");
        else
            LOG_PRINT("Missing texture created ({}x{})", SIZE, SIZE);

        return TextureHandle(asset);
    }
}