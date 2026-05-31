// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "../assets_types.hpp"
#include <filesystem>
#include <memory>

namespace Funkin::Assets::Loaders {
    std::shared_ptr<AudioClip> loadAudio(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group
    );
}