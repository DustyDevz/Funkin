// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>
#include "shader_compiler.hpp"
#include "filesystem/filesystem.hpp"

namespace Funkin::Shader {
    std::string rendererSuffix() {
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Vulkan:     return "spv";
            case bgfx::RendererType::OpenGL:     return "glsl";
            case bgfx::RendererType::Direct3D11: return "dx11";
            case bgfx::RendererType::Direct3D12: return "dx12";
            case bgfx::RendererType::Metal:      return "metal";
            default:                             return "glsl";
        }
    }

    std::string shaderProfile() {
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Vulkan:     return "spirv";
            case bgfx::RendererType::Direct3D11: return "vs_5_0";
            case bgfx::RendererType::Direct3D12: return "vs_6_0";
            case bgfx::RendererType::Metal:      return "metal";
            default:                             return "120";
        }
    }

    bool compileShaderToDisk(const char* source, const char* type, const std::string& outPath) {
        std::filesystem::create_directories(
            Funkin::Filesystem::resolve("cache://shaders"));

        std::string sType   = (std::string(type) == "fragment") ? "fragment" : "vertex";
        std::string profile = shaderProfile();

        if (sType == "fragment") {
            if (profile == "vs_5_0") profile = "ps_5_0";
            if (profile == "vs_6_0") profile = "ps_6_0";
        }

        auto tempSrcPath = std::filesystem::path(
            Funkin::Filesystem::resolve("cache://shaders/temp_" + sType + ".sc")).make_preferred();

        std::ofstream tempFile(tempSrcPath.string(), std::ios::trunc);
        if (!tempFile) {
            LOG_ERR("Failed to write temp shader source: {}", tempSrcPath.string());
            return false;
        }
        tempFile << source;
        tempFile.close();

        auto toolsExePath = (std::filesystem::path(SDL_GetBasePath()) / "bin" / "shaderc.exe").make_preferred();
        if (!std::filesystem::exists(toolsExePath)) {
            LOG_ERR("shaderc.exe not found at: {}", toolsExePath.string());
            return false;
        }

        auto bgfxInclude = (std::filesystem::path(SDL_GetBasePath()) / "bin" / "shaderc_include").make_preferred();
        auto varyingDef  = (std::filesystem::path(SDL_GetBasePath()) / "bin" / "varying.def.sc").make_preferred();
        auto outNormal   = std::filesystem::path(outPath).make_preferred();

        std::string command =
            "\"\"" + toolsExePath.string() + "\""
            " -f \""       + tempSrcPath.string() + "\""
            " -o \""       + outNormal.string()   + "\""
            " --type "     + sType                +
            " --platform windows"
            " -p "         + profile              +
            " --varyingdef \"" + varyingDef.string() + "\""
            " -i \""       + bgfxInclude.string() + "\"\"";

            static std::atomic<int> s_logCount{0};
            LOG_PRINT("Compiling {} shader: {} (ID: {})", sType, outNormal.string(), s_logCount++);

        int result = std::system(command.c_str());
        std::remove(tempSrcPath.string().c_str());

        if (result != 0) {
            LOG_ERR("Shader compile failed (exit {}): {}", result, outNormal.string());
            return false;
        }

        return true;
    }
}