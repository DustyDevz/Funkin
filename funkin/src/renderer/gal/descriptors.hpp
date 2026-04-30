// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "types.hpp"
#include "handles.hpp"
#include <math/vec2.hpp>
#include <math/vec4.hpp>
#include <math/color.hpp>
#include <math/rect.hpp>

namespace Funkin::Renderer::GAL {

struct GALDesc {
    void*    windowHandle  = nullptr;
    uint32_t width         = 1280;
    uint32_t height        = 720;
    bool     vsync         = false;
    uint32_t frameCount    = 2;
    PixelFormat swapFormat = PixelFormat::BGRA8_Unorm;
};

struct TextureDesc {
    uint32_t     width   = 1;
    uint32_t     height  = 1;
    uint32_t     mips    = 1;
    uint32_t     samples = 1;
    PixelFormat  format  = PixelFormat::RGBA8_Unorm;
    TextureUsage usage   = TextureUsage::Sampled;
};

struct BufferDesc {
    size_t      size   = 0;
    BufferUsage usage  = BufferUsage::Vertex;
    MemoryHint  memory = MemoryHint::GPU;
};

struct ShaderDesc {
    const uint8_t* bytecode = nullptr;
    size_t         size     = 0;
    ShaderStage    stage    = ShaderStage::Vertex;
};

struct VertexAttribute {
    const char* semantic = nullptr;
    uint32_t    index    = 0;
    PixelFormat format   = PixelFormat::RG32_Float;
    uint32_t    offset   = 0;
};

struct VertexLayout {
    const VertexAttribute* attributes = nullptr;
    uint32_t               count      = 0;
    uint32_t               stride     = 0;
};

struct DepthDesc {
    bool      testEnabled  = false;
    bool      writeEnabled = false;
    CompareOp compare      = CompareOp::Less;
};

struct RasterDesc {
    CullMode cullMode = CullMode::Back;
    FillMode fillMode = FillMode::Solid;
};

struct PipelineDesc {
    ShaderHandle     vs;
    ShaderHandle     ps;
    VertexLayout     vertexLayout;
    BlendMode        blend             = BlendMode::None;
    DepthDesc        depth;
    RasterDesc       raster;
    PrimitiveTopology topology         = PrimitiveTopology::TriangleList;
    PixelFormat      renderTargetFmt   = PixelFormat::BGRA8_Unorm;
    PixelFormat      depthStencilFmt   = PixelFormat::Unknown;
    uint32_t         renderTargetCount = 1;
};

struct SamplerDesc {
    FilterMode filter      = FilterMode::Linear;
    WrapMode   wrapU       = WrapMode::Clamp;
    WrapMode   wrapV       = WrapMode::Clamp;
    WrapMode   wrapW       = WrapMode::Clamp;
    float      anisotropy  = 1.0f;
    float      minLod      = 0.0f;
    float      maxLod      = 1000.0f;
    Color      borderColor = Color::transparent();
};

struct ColorAttachment {
    TextureHandle texture;
    LoadOp        loadOp      = LoadOp::Clear;
    StoreOp       storeOp     = StoreOp::Store;
    Color         clearColor  = { 0.0f, 0.0f, 0.0f, 1.0f };
};

struct DepthAttachment {
    TextureHandle texture;
    LoadOp        loadOp      = LoadOp::Clear;
    StoreOp       storeOp     = StoreOp::Store;
    float         clearDepth  = 1.0f;
    uint8_t       clearStencil = 0;
};

struct RenderPassDesc {
    const ColorAttachment* colorAttachments    = nullptr;
    uint32_t               colorCount          = 0;
    const DepthAttachment* depthAttachment      = nullptr;
    bool                   useSwapchainTarget  = false;
};

struct Viewport {
    float x      = 0.0f;
    float y      = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;
    float minZ   = 0.0f;
    float maxZ   = 1.0f;
};

struct DrawCmd {
    uint32_t vertexCount   = 0;
    uint32_t instanceCount = 1;
    uint32_t firstVertex   = 0;
    uint32_t firstInstance = 0;
};

struct DrawIndexedCmd {
    uint32_t indexCount    = 0;
    uint32_t instanceCount = 1;
    uint32_t firstIndex    = 0;
    int32_t  vertexOffset  = 0;
    uint32_t firstInstance = 0;
};

}
