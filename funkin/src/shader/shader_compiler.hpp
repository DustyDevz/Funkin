// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "shader_types.hpp"
#include <string>
#include <filesystem>

namespace Funkin::Renderer::Shader {
    class ShaderCompiler {
    public:
        static ShaderCode compileDX12(
            const std::filesystem::path& path,
            const std::string&           entryPoint,
            ShaderStage                  stage
        );

        static ShaderCode compileVulkan(
            const std::filesystem::path& path,
            ShaderStage                  stage
        );

        static ShaderPack compileFolder(
            const std::filesystem::path& folder,
            ShaderBackend                backend
        );

    private:
        static std::string stageToProfileDX12(ShaderStage stage);
        static std::string loadFile(const std::filesystem::path& path);
    };
}