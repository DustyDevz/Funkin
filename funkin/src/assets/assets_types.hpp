// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "assets_handle.hpp"
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <vector>

namespace Funkin::Assets {
    struct Texture : AssetBase {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        uint32_t width = 0;
        uint32_t height = 0;

        ~Texture() noexcept override {
            if (bgfx::isValid(handle)) {
                bgfx::destroy(handle);
            }
        }
    };

    struct AudioClip : AssetBase {
        std::vector<uint8_t> pcmData;
        uint32_t             sampleRate  = 44100;
        uint32_t             channels    = 2;
        uint64_t             frameCount  = 0;
    };

    struct Font : AssetBase {
        std::vector<uint8_t> ttfData;
    };

    struct JsonAsset : AssetBase {
        nlohmann::json data;
    };

    struct RawAsset : AssetBase {
        std::vector<uint8_t> bytes;
    };

    using TextureHandle  = AssetHandle<Texture>;
    using AudioHandle    = AssetHandle<AudioClip>;
    using FontHandle     = AssetHandle<Font>;
    using JsonHandle     = AssetHandle<JsonAsset>;
    using RawHandle      = AssetHandle<RawAsset>;
}