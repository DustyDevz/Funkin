// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "shader_types.hpp"
#include <filesystem>

namespace Funkin::Renderer::Shader {
    class ShaderPacker {
    public:
        static void pack(
            const ShaderPack&            pack,
            const std::filesystem::path& outputPath
        );

        static ShaderPack unpack(const std::filesystem::path& path);

    private:
        static constexpr uint32_t MAGIC   = 0x464B5348; // FKSH
        static constexpr uint8_t  XOR_KEY = 0x4B;

        static void   xorBuffer(std::vector<uint8_t>& buf);
        static size_t compress(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst);
        static bool   decompress(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst,
                                size_t originalSize);
    };
}