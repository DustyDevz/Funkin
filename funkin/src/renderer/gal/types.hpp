// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Renderer::GAL {
    enum class PixelFormat {
        Unknown,
        RGBA8_Unorm,
        RGBA8_Srgb,
        BGRA8_Unorm,
        BGRA8_Srgb,
        RGBA16_Float,
        RG16_Float,
        R16_Float,
        R32_Float,
        RG32_Float,
        RGB32_Float,
        RGBA32_Float,
        D32_Float,
        D24_Unorm_S8_Uint,
        D16_Unorm,
    };

    enum class BufferUsage : uint32_t {
        None    = 0,
        Vertex  = 1 << 0,
        Index   = 1 << 1,
        Uniform = 1 << 2,
        Staging = 1 << 3,
        Storage = 1 << 4,
    };

    inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
        return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline bool operator&(BufferUsage a, BufferUsage b) {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    enum class MemoryHint {
        GPU,
        CPUWrite,
        CPURead,
    };

    enum class TextureUsage : uint32_t {
        None         = 0,
        Sampled      = 1 << 0,
        RenderTarget = 1 << 1,
        DepthStencil = 1 << 2,
        Storage      = 1 << 3,
    };

    inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
        return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline bool operator&(TextureUsage a, TextureUsage b) {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    enum class ShaderStage {
        Vertex,
        Pixel,
        Compute,
        Geometry,
    };

    enum class PrimitiveTopology {
        TriangleList,
        TriangleStrip,
        LineList,
        LineStrip,
        PointList,
    };

    enum class CompareOp {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always,
    };

    enum class BlendMode {
        None,
        Alpha,
        Additive,
        Multiply,
        PremultipliedAlpha,
    };

    enum class FilterMode {
        Nearest,
        Linear,
        Anisotropic,
    };

    enum class WrapMode {
        Clamp,
        Repeat,
        Mirror,
        Border,
    };

    enum class CullMode {
        None,
        Front,
        Back,
    };

    enum class FillMode {
        Solid,
        Wireframe,
    };

    enum class IndexType {
        Uint16,
        Uint32,
    };

    enum class LoadOp {
        Load,
        Clear,
        DontCare,
    };

    enum class StoreOp {
        Store,
        DontCare,
    };
}
