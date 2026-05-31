// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>

namespace Funkin::Shader {
    struct ShaderSource {
        const char* name;
        const char* vs; // vertex
        const char* fs; // fragment
    };

    bgfx::ProgramHandle loadOrCompileProgram(const ShaderSource& source);
}