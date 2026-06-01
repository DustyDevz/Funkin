// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <array>

namespace Funkin::Shader {
    class Uniform {
    public:
        Uniform() = default;
        Uniform(const std::string& name, bgfx::UniformType::Enum type = bgfx::UniformType::Vec4);
        ~Uniform();

        Uniform(const Uniform&)            = delete;
        Uniform& operator=(const Uniform&) = delete;
        Uniform(Uniform&&)                 = default;
        Uniform& operator=(Uniform&&)      = default;

        void set(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
        void set(const float* data, uint16_t count = 1);
        void setMat4(const float* mtx);

        bool isValid() const { return bgfx::isValid(m_handle); }
        bgfx::UniformHandle handle() const { return m_handle; }

    private:
        bgfx::UniformHandle m_handle { BGFX_INVALID_HANDLE };
    };
}