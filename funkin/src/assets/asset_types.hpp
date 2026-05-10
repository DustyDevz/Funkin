// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Assets {
    static constexpr uint32_t FUNKIN_MAGIC   = 0x4B4E5546;
    static constexpr uint32_t FUNKIN_VERSION = 2;
    static constexpr uint8_t  XOR_KEY        = 0x4B;
    static constexpr uint8_t  XOR_SALT       = 0xA7;

    enum class AssetType : uint8_t {
        Unknown  = 0,
        Texture  = 1,
        Audio    = 2,
        Font     = 3,
        Binary   = 4,
        Shader   = 5,
    };

    enum class AssetSubtype : uint8_t {
        None         = 0,

        ShaderVertex = 1,
        ShaderPixel  = 2,
        ShaderFrag   = 3,
        ShaderCompute= 4,

        TexPNG       = 10,
        TexJPG       = 11,
        TexTGA       = 12,
        TexBMP       = 13,
        TexDDS       = 14,
        TexHDR       = 15,

        AudioWAV     = 20,
        AudioOGG     = 21,
        AudioMP3     = 22,
        AudioFLAC    = 23,
    };

    enum class ShaderBackend : uint8_t {
        DX12   = 0,
        Vulkan = 1,
    };

    enum class CompressMode : uint8_t {
        None    = 0,
        Fast    = 1,
        Default = 2,
        Max     = 3,
    };

    struct AssetID {
        uint64_t hash = 0;
        bool operator==(const AssetID& o) const { return hash == o.hash; }
        bool operator!=(const AssetID& o) const { return hash != o.hash; }
        bool valid() const { return hash != 0; }
    };

    struct AssetIDHasher {
        size_t operator()(const AssetID& id) const { return (size_t)id.hash; }
    };

    struct AssetFingerprint {
        uint64_t mtime     = 0;
        uint64_t fileSize  = 0;
    };

    struct AssetTOCEntry {
        AssetID         id;
        AssetType       type;
        AssetSubtype    subtype;
        CompressMode    compressMode;
        ShaderBackend   shaderBackend;
        uint8_t         _pad[3];
        uint64_t        offset;
        uint32_t        compressedSize;
        uint32_t        originalSize;
        AssetFingerprint fingerprint;
        char            path[256];
    };

    struct AssetPackHeader {
        uint32_t magic;
        uint32_t version;
        uint32_t entryCount;
        uint32_t _pad;
        uint64_t tocOffset;
        uint64_t buildTime;
    };

    struct TextureAsset {
        uint32_t             width    = 0;
        uint32_t             height   = 0;
        uint32_t             channels = 4;
        bool                 isDDS    = false;
        std::vector<uint8_t> pixels;
    };

    struct AudioAsset {
        uint32_t             sampleRate     = 44100;
        uint16_t             channels       = 2;
        uint16_t             bitsPerSample  = 16;
        bool                 isCompressed   = false;
        std::vector<uint8_t> data;
    };

    struct FontAsset {
        std::vector<uint8_t> ttfData;
    };

    struct ShaderAsset {
        std::vector<uint8_t> bytecode;
        AssetSubtype         stage;
        ShaderBackend        backend;
        std::string          name;
        bool empty() const { return bytecode.empty(); }
    };

    struct BinaryAsset {
        std::vector<uint8_t> data;
    };

    AssetID      makeAssetID(std::string_view path);
    AssetID      makeShaderID(std::string_view name, AssetSubtype stage);
    AssetType    inferAssetType(std::string_view ext);
    AssetSubtype inferAssetSubtype(std::string_view ext);
    CompressMode pickCompressMode(AssetType type, AssetSubtype subtype);
    int          compressModeToZstdLevel(CompressMode mode);
}