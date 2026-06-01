// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <string>
#include <vector>

namespace Funkin::Shader {
    enum class ShaderInputType {
        Shadertoy, // https://www.shadertoy.com/
        GLSL,
        HLSL,
        SC
    };

    struct ConvertedShader {
        std::string vs;
        std::string fs;
        std::vector<std::string> uniforms;
        ShaderInputType sourceType;
    };

    ConvertedShader convertShader(const std::string& source, ShaderInputType type);
}   