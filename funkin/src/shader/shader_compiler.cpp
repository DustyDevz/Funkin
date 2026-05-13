// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "shader_compiler.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <wrl/client.h>

#ifdef _WIN32
    #include <d3dcompiler.h>
    #pragma comment(lib, "d3dcompiler.lib")
#endif

using Microsoft::WRL::ComPtr;
namespace Funkin::Renderer::Shader {
    std::string ShaderCompiler::loadFile(const std::filesystem::path& path) {
        std::ifstream f(path);
        if (!f.is_open())
            throw std::runtime_error("Failed to open shader: " + path.string());

        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    std::string ShaderCompiler::stageToProfileDX12(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:   return "vs_5_0";
            case ShaderStage::Pixel:
            case ShaderStage::Fragment: return "ps_5_0";
            case ShaderStage::Compute:  return "cs_5_0";
            default: throw std::runtime_error("Unknown shader stage");
        }
    }

    #ifdef _WIN32
        ShaderCode ShaderCompiler::compileDX12(
            const std::filesystem::path& path,
            const std::string&           entryPoint,
            ShaderStage                  stage) 
        {
            std::string source  = loadFile(path);
            std::string profile = stageToProfileDX12(stage);

            UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
            #ifdef FUNKIN_DEBUG
                flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
            #else
                flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
            #endif

            ComPtr<ID3DBlob> blob; ComPtr<ID3DBlob> error;

            HRESULT hr = D3DCompile(
                source.c_str(), source.size(),
                path.string().c_str(),
                nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                entryPoint.c_str(), profile.c_str(),
                flags, 0, &blob, &error
            );

            // Can probably do something better but eh
            if (FAILED(hr)) {
                std::string msg = "Shader compile error: ";
                if (error) msg += (char*)error->GetBufferPointer();
                throw std::runtime_error(msg);
            }

            ShaderCode code;
            code.stage = stage;
            code.backend = ShaderBackend::DX12;
            code.name = path.stem().string();
            code.bytecode.resize(blob->GetBufferSize());
            memcpy(code.bytecode.data(), blob->GetBufferPointer(), blob->GetBufferSize());
            return code;
        }
    #endif

    ShaderCode ShaderCompiler::compileVulkan(
        const std::filesystem::path& path,
        ShaderStage                  stage)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open())
            throw std::runtime_error("Failed to open SPIR-V: " + path.string());

        ShaderCode code;
        code.stage   = stage;
        code.backend = ShaderBackend::Vulkan;
        code.name    = path.stem().string();
        code.bytecode = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>()
        );
        return code;
    }

    ShaderPack ShaderCompiler::compileFolder(
        const std::filesystem::path& folder,
        ShaderBackend                backend)
    {
        ShaderPack pack;

        for (auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
            if (!entry.is_regular_file()) continue;

            auto ext = entry.path().extension().string();

            #ifdef _WIN32
                if (backend == ShaderBackend::DX12 && ext == ".hlsl") {
                    pack.shaders.push_back(compileDX12(entry.path(), "VSMain", ShaderStage::Vertex));
                    pack.shaders.push_back(compileDX12(entry.path(), "PSMain", ShaderStage::Pixel));
                }
            #endif
            if (backend == ShaderBackend::Vulkan && ext == ".spv") {
                ShaderStage stage = ShaderStage::Vertex;
                auto stem = entry.path().stem().string();
                if (stem.ends_with(".frag")) stage = ShaderStage::Fragment;
                pack.shaders.push_back(compileVulkan(entry.path(), stage));
            }
        }

        return pack;
    }
}