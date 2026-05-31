// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <fstream>
#include <nlohmann/json.hpp>
#include "json_loader.hpp"

namespace Funkin::Assets::Loaders {
    std::shared_ptr<JsonAsset> loadJson(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {

        std::ifstream f(path);
        if (!f.is_open()) {
            LOG_ERR("JSON open failed: {}", id);
            return nullptr;
        }

        auto asset    = std::make_shared<JsonAsset>();
        asset->id     = id;
        asset->group  = group;

        try {
            asset->data      = nlohmann::json::parse(f, nullptr, true, true);
            asset->sizeBytes = std::filesystem::file_size(path);
            asset->loaded    = true;
            LOG_PRINT("JSON loaded: {} ({} bytes)", id, asset->sizeBytes);
        } catch (const nlohmann::json::exception& ex) {
            LOG_ERR("JSON parse failed: {} — {}", id, ex.what());
            return nullptr;
        }

        return asset;
    }
}