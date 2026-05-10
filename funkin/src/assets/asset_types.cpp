// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "asset_types.hpp"

namespace Funkin::Assets {
    AssetID makeAssetID(std::string_view path) {
        AssetID id;
        id.hash = 14695981039346656037ULL;
        for (char c : path) {
            id.hash ^= (uint8_t)c;
            id.hash *= 1099511628211ULL;
        }
        return id;
    }

    AssetID makeShaderID(std::string_view name, AssetSubtype stage) {
        std::string key = std::string(name) + "_" + std::to_string((uint8_t)stage);
        return makeAssetID(key);
    }

    AssetType inferAssetType(std::string_view ext) {
        if (ext == "png" || ext == "jpg" || ext == "jpeg" ||
            ext == "tga" || ext == "bmp" || ext == "dds"  || ext == "hdr")
            return AssetType::Texture;
        if (ext == "wav" || ext == "ogg" || ext == "mp3"  || ext == "flac")
            return AssetType::Audio;
        if (ext == "ttf" || ext == "otf")
            return AssetType::Font;
        if (ext == "hlsl" || ext == "spv")
            return AssetType::Shader;
        return AssetType::Binary;
    }

    AssetSubtype inferAssetSubtype(std::string_view ext) {
        if (ext == "png")  return AssetSubtype::TexPNG;
        if (ext == "jpg" || ext == "jpeg") return AssetSubtype::TexJPG;
        if (ext == "tga")  return AssetSubtype::TexTGA;
        if (ext == "bmp")  return AssetSubtype::TexBMP;
        if (ext == "dds")  return AssetSubtype::TexDDS;
        if (ext == "hdr")  return AssetSubtype::TexHDR;
        if (ext == "wav")  return AssetSubtype::AudioWAV;
        if (ext == "ogg")  return AssetSubtype::AudioOGG;
        if (ext == "mp3")  return AssetSubtype::AudioMP3;
        if (ext == "flac") return AssetSubtype::AudioFLAC;
        return AssetSubtype::None;
    }

    CompressMode pickCompressMode(AssetType type, AssetSubtype subtype) {
        switch (type) {
            case AssetType::Shader: return CompressMode::Max;
            case AssetType::Font:   return CompressMode::Max;
            case AssetType::Binary: return CompressMode::Max;
            case AssetType::Audio:
                // SAVE THE CPU!!
                if (subtype == AssetSubtype::AudioOGG ||
                    subtype == AssetSubtype::AudioMP3)
                    return CompressMode::None;
                return CompressMode::Default;

            case AssetType::Texture:
                if (subtype == AssetSubtype::TexPNG ||
                    subtype == AssetSubtype::TexJPG)
                    return CompressMode::None;
                if (subtype == AssetSubtype::TexDDS)
                    return CompressMode::Fast;
                return CompressMode::Default;

            default:
                return CompressMode::Default;
        }
    }

    int compressModeToZstdLevel(CompressMode mode) {
        switch (mode) {
            case CompressMode::None:    return 0;
            case CompressMode::Fast:    return 1;
            case CompressMode::Default: return 9;
            case CompressMode::Max:     return 19;
        }
        return 9;
    }
}