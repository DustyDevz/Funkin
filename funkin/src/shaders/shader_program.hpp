// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "shader_converter.hpp"
#include "shader_uniform.hpp"

namespace Funkin::Shader {
    class Program {
    public:
        Program() = default;
        Program(const std::string& name, const char* vs, const char* fs, ShaderInputType type = ShaderInputType::SC);
        ~Program();

        Program(const Program&)            = delete;
        Program& operator=(const Program&) = delete;
        Program(Program&&)                 = default;
        Program& operator=(Program&&)      = default;

        void recompile(const std::string& fs, ShaderInputType type = ShaderInputType::Shadertoy);
        void recompile(const std::string& vs, const std::string& fs, ShaderInputType type = ShaderInputType::SC);

        Uniform& uniform(const std::string& name, bgfx::UniformType::Enum type = bgfx::UniformType::Vec4);

        bool                isValid()  const { return bgfx::isValid(m_handle); }
        bgfx::ProgramHandle handle()   const { return m_handle; }

    private:
        std::string          m_name;
        bgfx::ProgramHandle  m_handle  { BGFX_INVALID_HANDLE };
        bgfx::ProgramHandle  m_pending { BGFX_INVALID_HANDLE };

        std::unordered_map<std::string, std::unique_ptr<Uniform>> m_uniforms;

        void submit(const std::string& vs, const std::string& fs, ShaderInputType type);

        friend void tickShaderJobs();
        friend class std::vector<Program*>;
    };
}