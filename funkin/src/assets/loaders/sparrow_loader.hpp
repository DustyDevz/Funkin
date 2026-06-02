// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "assets/assets_types.hpp"
#include <filesystem>
#include <memory>

namespace Funkin::Assets::Loaders {
    std::shared_ptr<SparrowAtlas> loadSparrow(
        const std::filesystem::path& xmlPath,
        const std::string& id,
        const std::string& group
    );
}