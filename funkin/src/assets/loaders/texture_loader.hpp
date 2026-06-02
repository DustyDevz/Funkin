// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <memory>
#include "../assets_types.hpp"

namespace Funkin::Assets::Loaders {
    struct PendingTextureUpload {
        std::string          id;
        std::string          group;
        std::vector<uint8_t> bytes;
        uint32_t             width;
        uint32_t             height;
        bool                 isKtx;
    };

    std::optional<PendingTextureUpload> prepareTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group
    );

    std::shared_ptr<Texture> uploadPendingTexture(PendingTextureUpload& pending);

    std::shared_ptr<Texture> loadTexture(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group
    );

    TextureHandle createMissingTexture();
}