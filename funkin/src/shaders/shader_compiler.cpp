// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>
#include "shader_compiler.hpp"
#include "filesystem/filesystem.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <cstdlib>

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include <xxhash.h>

namespace Funkin::Shader {
    static std::string rendererSuffix() {
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Vulkan:     return "spv";
            case bgfx::RendererType::OpenGL:     return "glsl";
            case bgfx::RendererType::Direct3D11: return "dx11";
            case bgfx::RendererType::Direct3D12: return "dx12";
            case bgfx::RendererType::Metal:      return "metal";
            default:                             return "glsl";
        }
    }

    static std::string shaderProfile() {
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Vulkan:     return "spirv";
            case bgfx::RendererType::Direct3D11: return "vs_5_0";
            case bgfx::RendererType::Direct3D12: return "vs_6_0";
            case bgfx::RendererType::Metal:      return "metal";
            default:                             return "120";
        }
    }

    static bgfx::ShaderHandle loadFromCache(const std::string& cachePath) {
        std::ifstream f(cachePath, std::ios::binary | std::ios::ate);
        if (!f) return BGFX_INVALID_HANDLE;

        std::streamsize size = f.tellg();
        f.seekg(0, std::ios::beg);

        std::vector<uint8_t> bytes(size);
        if (!f.read(reinterpret_cast<char*>(bytes.data()), size)) {
            return BGFX_INVALID_HANDLE;
        }

        if (bytes.empty()) return BGFX_INVALID_HANDLE;

        const bgfx::Memory* mem = bgfx::copy(bytes.data(), (uint32_t)bytes.size());
        return bgfx::createShader(mem);
    }

    static bool compileShader(
        const char* source,
        const char* type,
        const std::string& outPath) {

        auto cacheDir = Funkin::Filesystem::resolve("cache://shaders");
        std::filesystem::create_directories(cacheDir);

        std::string sType = (std::string(type) == "fragment") ? "fragment" : "vertex";
        std::string profile = shaderProfile();

        auto tempSrcPath = std::filesystem::path(
            Funkin::Filesystem::resolve("cache://shaders/temp_" + sType + ".sc")).make_preferred();
        std::ofstream tempFile(tempSrcPath.string(), std::ios::trunc);
        if (!tempFile) {
            LOG_ERR("Failed to write temp shader source: {}", tempSrcPath.string());
            return false;
        }
        tempFile << source;
        tempFile.close();

        if (sType == "fragment") {
            if (profile == "vs_5_0") profile = "ps_5_0";
            if (profile == "vs_6_0") profile = "ps_6_0";
        }

        auto toolsExePath = std::filesystem::path(SDL_GetBasePath()) / "bin" / "shaderc.exe";

        if (!std::filesystem::exists(toolsExePath)) {
            LOG_ERR("shaderc.exe not found at: {}", toolsExePath.string());
            return false;
        }

        auto bgfxInclude = (std::filesystem::path(SDL_GetBasePath()) / "bin" / "shaderc_include").make_preferred();
        auto varyingDef  = (std::filesystem::path(SDL_GetBasePath()) / "bin" / "varying.def.sc").make_preferred();

        std::string command =
            "\"\"" + toolsExePath.string() + "\""
            " -f \"" + tempSrcPath.string() + "\""
            " -o \"" + std::filesystem::path(outPath).make_preferred().string() + "\""
            " --type " + sType +
            " --platform windows"
            " -p " + profile +
            " --varyingdef \"" + varyingDef.string() + "\""
            " -i \"" + bgfxInclude.string() + "\"\"";

        LOG_PRINT("Compiling {} shader: {}", sType, outPath);

        LOG_PRINT("Command: {}", command);
        int result = std::system(command.c_str());
        std::remove(tempSrcPath.string().c_str());

        if (result != 0) {
            LOG_ERR("Shader compile failed (exit {}): {}", result, outPath);
            return false;
        }

        return true;
    }

    bgfx::ProgramHandle loadOrCompileProgram(const ShaderSource& src) {
        auto suffix     = rendererSuffix();
        uint64_t vsHash = XXH3_64bits(src.vs, strlen(src.vs));
        uint64_t fsHash = XXH3_64bits(src.fs, strlen(src.fs));

        auto vsCache = std::filesystem::path(Funkin::Filesystem::resolve(
            std::format("cache://shaders/{}_vs_{:x}_{}.bin", src.name, vsHash, suffix))).make_preferred();
        auto fsCache = std::filesystem::path(Funkin::Filesystem::resolve(
            std::format("cache://shaders/{}_fs_{:x}_{}.bin", src.name, fsHash, suffix))).make_preferred();

        LOG_PRINT("Loading shader: {} ({})", src.name, suffix);

        bool vsHit = bgfx::isValid(loadFromCache(vsCache.string()));
        bool fsHit = bgfx::isValid(loadFromCache(fsCache.string()));

        if (!vsHit || !fsHit) {
            auto vsFuture = std::async(std::launch::async, [&]() -> bool {
                if (vsHit) return true;
                LOG_PRINT("Cache miss VS: {}", vsCache.string());
                return compileShader(src.vs, "vertex", vsCache.string());
            });

            auto fsFuture = std::async(std::launch::async, [&]() -> bool {
                if (fsHit) return true;
                LOG_PRINT("Cache miss FS: {}", fsCache.string());
                return compileShader(src.fs, "fragment", fsCache.string());
            });

            if (!vsFuture.get() || !fsFuture.get())
                return BGFX_INVALID_HANDLE;
        }

        bgfx::ShaderHandle vs = loadFromCache(vsCache.string());
        bgfx::ShaderHandle fs = loadFromCache(fsCache.string());

        if (!bgfx::isValid(vs)) {
            LOG_ERR("Compiled VS loaded but invalid: {}", src.name);
            return BGFX_INVALID_HANDLE;
        }
        if (!bgfx::isValid(fs)) {
            LOG_ERR("Compiled FS loaded but invalid: {}", src.name);
            return BGFX_INVALID_HANDLE;
        }

        LOG_PRINT("Shader program created: {}", src.name);
        return bgfx::createProgram(vs, fs, true);
    }
}