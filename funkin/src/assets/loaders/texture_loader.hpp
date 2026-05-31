// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <memory>
#include "../assets_types.hpp"

namespace Funkin::Assets::Loaders {
    std::shared_ptr<Texture> loadTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group
    );

    TextureHandle createMissingTexture();
}