// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "assets_handle.hpp"
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <unordered_map>

namespace Funkin::Assets {
    struct Texture : AssetBase {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        uint32_t width = 0;
        uint32_t height = 0;

        ~Texture() noexcept override {
            if (bgfx::isValid(handle))
                bgfx::destroy(handle);
        }
    };

    struct AudioClip : AssetBase {
        std::vector<uint8_t> pcmData;
        uint32_t             sampleRate = 44100;
        uint32_t             channels   = 2;
        uint64_t             frameCount = 0;
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

    struct SparrowFrame {
        std::string name;
        float x      = 0, y      = 0;
        float w      = 0, h      = 0;
        float frameX = 0, frameY = 0;
        float frameW = 0, frameH = 0;
    };

    struct SparrowAtlas : AssetBase {
        AssetHandle<Texture>                    texture;
        std::vector<SparrowFrame>               frames;
        std::unordered_map<std::string, size_t> frameIndex;

        std::vector<size_t> framesWithPrefix(const std::string& prefix) const {
            std::vector<size_t> result;
            for (size_t i = 0; i < frames.size(); i++)
                if (frames[i].name.rfind(prefix, 0) == 0)
                    result.push_back(i);
            return result;
        }
    };

    using TextureHandle  = AssetHandle<Texture>;
    using AudioHandle    = AssetHandle<AudioClip>;
    using FontHandle     = AssetHandle<Font>;
    using JsonHandle     = AssetHandle<JsonAsset>;
    using RawHandle      = AssetHandle<RawAsset>;
    using SparrowHandle  = AssetHandle<SparrowAtlas>;
}