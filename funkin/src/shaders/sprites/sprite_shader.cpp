// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "sprite_shader.hpp"
#include "shaders/shader_program.hpp"

namespace Funkin::Shader::Sprites {
    static Funkin::Shader::Program* s_program = nullptr;

    void init() {
        s_program = new Funkin::Shader::Program(
            "sprite",
            SpriteVS,
            SpriteFS
        );
    }

    void shutdown() {
        delete s_program;
        s_program = nullptr;
    }

    bgfx::ProgramHandle programHandle() {
        if (!s_program || !s_program->isValid())
            return BGFX_INVALID_HANDLE;
        return s_program->handle();
    }
}