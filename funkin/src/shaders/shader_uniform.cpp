// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "shader_uniform.hpp"

namespace Funkin::Shader {
    Uniform::Uniform(const std::string& name, bgfx::UniformType::Enum type) {
        m_handle = bgfx::createUniform(name.c_str(), type);
    }

    Uniform::~Uniform() {
        if (bgfx::isValid(m_handle))
            bgfx::destroy(m_handle);
    }

    void Uniform::set(float x, float y, float z, float w) {
        float data[4] = { x, y, z, w };
        bgfx::setUniform(m_handle, data);
    }

    void Uniform::set(const float* data, uint16_t count) {
        bgfx::setUniform(m_handle, data, count);
    }

    void Uniform::setMat4(const float* mtx) {
        bgfx::setUniform(m_handle, mtx);
    }
}