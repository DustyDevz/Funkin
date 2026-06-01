// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>

namespace Funkin::Shader {
    std::string rendererSuffix();
    std::string shaderProfile();
    bool compileShaderToDisk(const char* source, const char* type, const std::string& outPath);
}