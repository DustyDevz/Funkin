// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "font_loader.hpp"
#include "shared/log.hpp"

namespace Funkin::Assets::Loaders {
    std::shared_ptr<Font> loadFont(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) {
            LOG_ERR("Font open failed: {}", id);
            return nullptr;
        }

        auto asset    = std::make_shared<Font>();
        asset->id     = id;
        asset->group  = group;
        asset->ttfData = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>()
        );

        if (asset->ttfData.empty()) {
            LOG_ERR("Font is empty: {}", id);
            return nullptr;
        }

        asset->sizeBytes = asset->ttfData.size();
        asset->loaded    = true;
        LOG_PRINT("Font loaded: {} ({} bytes)", id, asset->sizeBytes);
        return asset;
    }
}