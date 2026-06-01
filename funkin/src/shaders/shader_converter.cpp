// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "shader_converter.hpp"

namespace Funkin::Shader {
    static const char* DEFAULT_VS = R"($input a_position, a_texcoord0
    $output v_texcoord0

    #include <bgfx_shader.sh>

    void main() {
        gl_Position = mul(u_modelViewProj, a_position);
        v_texcoord0 = a_texcoord0;
    }
    )";

    static std::string buildFSHeader(const std::vector<std::string>& extraUniforms) {
        std::string header = "$input v_texcoord0\n\n#include <bgfx_shader.sh>\n\n";
        for (const auto& u : extraUniforms)
            header += "uniform vec4 " + u + ";\n";
        if (!extraUniforms.empty())
            header += "\n";
        return header;
    }

    static std::vector<std::string> detectShadertoyUniforms(const std::string& src) {
        std::vector<std::string> uniforms;
        if (src.find("iTime") != std::string::npos)         uniforms.push_back("u_time");
        if (src.find("iResolution") != std::string::npos)   uniforms.push_back("u_resolution");
        if (src.find("iMouse") != std::string::npos)        uniforms.push_back("u_mouse");
        if (src.find("iFrame") != std::string::npos)        uniforms.push_back("u_frame");
        if (src.find("iTimeDelta") != std::string::npos)    uniforms.push_back("u_timeDelta");
        return uniforms;
    }

    static ConvertedShader convertShadertoy(const std::string& src) {
        ConvertedShader out;
        out.sourceType = ShaderInputType::Shadertoy;
        out.vs = DEFAULT_VS;
        out.uniforms = detectShadertoyUniforms(src);

        std::string fs = src;

        fs = std::regex_replace(fs,
            std::regex(R"(void\s+mainImage\s*\(\s*out\s+vec4\s+(\w+)\s*,\s*in\s+vec2\s+(\w+)\s*\))"),
            "void main()");

        fs = std::regex_replace(fs, std::regex(R"(\bfragColor\b)"),   "gl_FragColor");
        fs = std::regex_replace(fs, std::regex(R"(\bfragCoord\b)"),   "(v_texcoord0 * u_resolution.xy)");
        fs = std::regex_replace(fs, std::regex(R"(\biTime\b)"),       "u_time.x");
        fs = std::regex_replace(fs, std::regex(R"(\biTimeDelta\b)"),  "u_timeDelta.x");
        fs = std::regex_replace(fs, std::regex(R"(\biResolution\b)"), "u_resolution");
        fs = std::regex_replace(fs, std::regex(R"(\biMouse\b)"),      "u_mouse");
        fs = std::regex_replace(fs, std::regex(R"(\biFrame\b)"),      "int(u_frame.x)");
        fs = std::regex_replace(fs, std::regex(R"(\btexture\s*\()"),  "texture2D(");

        out.fs = buildFSHeader(out.uniforms) + fs;
        return out;
    }

    static ConvertedShader convertGLSL(const std::string& src) {
        ConvertedShader out;
        out.sourceType = ShaderInputType::GLSL;
        out.vs = DEFAULT_VS;

        std::string fs = src;
        fs = std::regex_replace(fs, std::regex(R"(#version\s+\d+(\s+\w+)?\n?)"), "");

        std::smatch m;
        std::regex outVarRe(R"(out\s+vec4\s+(\w+)\s*;)");
        std::string outVar;
        if (std::regex_search(fs, m, outVarRe)) {
            outVar = m[1].str();
            fs = std::regex_replace(fs, outVarRe, "");
            if (!outVar.empty())
                fs = std::regex_replace(fs, std::regex("\\b" + outVar + "\\b"), "gl_FragColor");
        }

        std::regex inVarRe(R"(in\s+vec2\s+(\w+)\s*;)");
        std::string inVar;
        if (std::regex_search(fs, m, inVarRe)) {
            inVar = m[1].str();
            fs = std::regex_replace(fs, inVarRe, "");
            if (!inVar.empty())
                fs = std::regex_replace(fs, std::regex("\\b" + inVar + "\\b"), "v_texcoord0");
        }

        fs = std::regex_replace(fs, std::regex(R"(\btexture\s*\()"), "texture2D(");

        out.uniforms = detectShadertoyUniforms(fs);
        out.fs = buildFSHeader(out.uniforms) + fs;
        return out;
    }

    static ConvertedShader convertHLSL(const std::string& src) {
        ConvertedShader out;
        out.sourceType = ShaderInputType::HLSL;
        out.vs = DEFAULT_VS;

        // one day.. trust
        LOG_ERR("HLSL conversion not yet supported");
        out.fs = src;
        return out;
    }

    static ConvertedShader passThroughSC(const std::string& src) {
        ConvertedShader out;
        out.sourceType = ShaderInputType::SC;
        out.vs = DEFAULT_VS;
        out.fs = src;
        return out;
    }

    ConvertedShader convertShader(const std::string& source, ShaderInputType type) {
        switch (type) {
            case ShaderInputType::Shadertoy: return convertShadertoy(source);
            case ShaderInputType::GLSL:      return convertGLSL(source);
            case ShaderInputType::HLSL:      return convertHLSL(source);
            case ShaderInputType::SC:        return passThroughSC(source);
        }
        return passThroughSC(source);
    }
}