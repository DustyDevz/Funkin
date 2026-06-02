// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>

namespace Funkin::Shader::Sprites {
    inline constexpr const char* SpriteVS = R"(
    $input a_position, a_texcoord0, a_color0
    $output v_texcoord0, v_color0

    #include <bgfx_shader.sh>

    void main() {
        gl_Position = mul(u_modelViewProj, vec4(a_position.x, a_position.y, a_position.z, 1.0));
        v_texcoord0 = a_texcoord0;
        v_color0    = a_color0;
    }
    )";

    inline constexpr const char* SpriteFS = R"(
    $input v_texcoord0, v_color0

    #include <bgfx_shader.sh>

    SAMPLER2D(s_tex, 0);

    void main() {
        vec4 texColor = texture2D(s_tex, v_texcoord0);
        gl_FragColor  = texColor * v_color0;
    }
    )";

    void                init();
    void                shutdown();
    bgfx::ProgramHandle programHandle();
}