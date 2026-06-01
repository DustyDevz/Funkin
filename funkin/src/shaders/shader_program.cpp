// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "shader_program.hpp"
#include "shader_job_queue.hpp"
#include "shader_converter.hpp"

namespace Funkin::Shader {
    Program::Program(const std::string& name, const char* vs, const char* fs, ShaderInputType type)
        : m_name(name) {
        registerProgram(this);
        submit(vs, fs, type);
    }

    Program::~Program() {
        unregisterProgram(this);
        if (bgfx::isValid(m_handle))  bgfx::destroy(m_handle);
        if (bgfx::isValid(m_pending)) bgfx::destroy(m_pending);
    }

    void Program::recompile(const std::string& fs, ShaderInputType type) {
        auto converted = convertShader(fs, type);
        submit(converted.vs, converted.fs, ShaderInputType::SC);
    }

    void Program::recompile(const std::string& vs, const std::string& fs, ShaderInputType type) {
        submit(vs, fs, type);
    }

    Uniform& Program::uniform(const std::string& name, bgfx::UniformType::Enum type) {
        auto it = m_uniforms.find(name);
        if (it == m_uniforms.end()) {
            m_uniforms[name] = std::make_unique<Uniform>(name, type);
            return *m_uniforms[name];
        }
        return *it->second;
    }

    void Program::submit(const std::string& vs, const std::string& fs, ShaderInputType type) {
        ConvertedShader converted;
        if (type == ShaderInputType::SC) {
            converted.vs         = vs;
            converted.fs         = fs;
            converted.sourceType = ShaderInputType::SC;
        } else {
            converted = convertShader(fs, type);
        }

        submitShaderJob({
            m_name,
            converted,
            [this](bgfx::ProgramHandle prog) {
                if (bgfx::isValid(prog)) {
                    if (bgfx::isValid(m_pending))
                        bgfx::destroy(m_pending);
                    m_pending = prog;
                } else {
                    LOG_ERR("Shader job returned invalid program: {}", m_name);
                }
            }
        });
    }
}