// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <functional>
#include "shader_converter.hpp"

namespace Funkin::Shader {
    class Program;

    struct ShaderJob {
        std::string name;
        ConvertedShader shader;
        std::function<void(bgfx::ProgramHandle)> onComplete;
    };

    void registerProgram(Program* p);
    void unregisterProgram(Program* p);
    void submitShaderJob(ShaderJob job);
    void tickShaderJobs();
    void shutdownShaderJobs();
}