// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "shader_packer.hpp"
#include <fstream>
#include <stdexcept>
#include <zstd.h>

namespace Funkin::Renderer::Shader {
    void ShaderPacker::xorBuffer(std::vector<uint8_t>& buf) {
        for (auto& b : buf) b ^= XOR_KEY;
    }

    size_t ShaderPacker::compress(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst) {
        size_t bound  = ZSTD_compressBound(src.size());
        dst.resize(bound);

        size_t result = ZSTD_compress(
            dst.data(), bound,
            src.data(), src.size(),
            3 // 1-22 (ill tweak with this more later)
        );

        if (ZSTD_isError(result))
            throw std::runtime_error(
                std::string("zstd compress failed: ") + ZSTD_getErrorName(result)
            );

        dst.resize(result);
        return result;
    }

    bool ShaderPacker::decompress(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst,
                                size_t originalSize) {

        dst.resize(originalSize);

        size_t result = ZSTD_decompress(
            dst.data(), originalSize,
            src.data(), src.size()
        );

        if (ZSTD_isError(result))
            return false;

        return true;
    }

    void ShaderPacker::pack(const ShaderPack& pack, const std::filesystem::path& outputPath) {
        std::filesystem::create_directories(outputPath.parent_path());
        std::ofstream f(outputPath, std::ios::binary);
        if (!f.is_open())
            throw std::runtime_error("Failed to open output: " + outputPath.string());

        uint32_t count = (uint32_t)pack.shaders.size();
        f.write((char*)&MAGIC, 4);
        f.write((char*)&count, 4);

        for (auto& shader : pack.shaders) {
            uint32_t nameLen = (uint32_t)shader.name.size();
            f.write((char*)&nameLen, 4);
            f.write(shader.name.c_str(), nameLen);

            uint8_t stage   = (uint8_t)shader.stage;
            uint8_t backend = (uint8_t)shader.backend;
            f.write((char*)&stage,   1);
            f.write((char*)&backend, 1);

            std::vector<uint8_t> compressed;
            uint32_t originalSize   = (uint32_t)shader.bytecode.size();
            uint32_t compressedSize = (uint32_t)compress(shader.bytecode, compressed);
            xorBuffer(compressed);

            f.write((char*)&originalSize,   4);
            f.write((char*)&compressedSize, 4);
            f.write((char*)compressed.data(), compressedSize);
        }
    }

    ShaderPack ShaderPacker::unpack(const std::filesystem::path& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open())
            throw std::runtime_error("Failed to open shader pack: " + path.string());

        uint32_t magic = 0, count = 0;
        f.read((char*)&magic, 4);
        f.read((char*)&count, 4);

        if (magic != MAGIC)
            throw std::runtime_error("Invalid shader pack file");

        ShaderPack pack;

        for (uint32_t i = 0; i < count; ++i) {
            ShaderCode code;

            uint32_t nameLen = 0;
            f.read((char*)&nameLen, 4);
            code.name.resize(nameLen);
            f.read(code.name.data(), nameLen);

            uint8_t stage = 0, backend = 0;
            f.read((char*)&stage,   1);
            f.read((char*)&backend, 1);
            code.stage   = (ShaderStage)stage;
            code.backend = (ShaderBackend)backend;

            uint32_t originalSize = 0, compressedSize = 0;
            f.read((char*)&originalSize,   4);
            f.read((char*)&compressedSize, 4);

            std::vector<uint8_t> compressed(compressedSize);
            f.read((char*)compressed.data(), compressedSize);

            xorBuffer(compressed);
            if (!decompress(compressed, code.bytecode, originalSize))
                throw std::runtime_error("Failed to decompress shader: " + code.name);

            pack.shaders.push_back(std::move(code));
        }

        return pack;
    }
}