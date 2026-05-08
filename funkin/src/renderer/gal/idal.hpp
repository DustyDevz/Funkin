// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "descriptors.hpp"
#include <math/vec2.hpp>
#include <math/vec3.hpp>
#include <math/mat4.hpp>
#include <math/color.hpp>

namespace Funkin::Renderer::GAL {
    class IDAL {
    public:
        virtual ~IDAL() = default;

        virtual bool init(const GALDesc& desc)     = 0;
        virtual void shutdown()                    = 0;

        virtual void beginFrame()                  = 0;
        virtual void endFrame()                    = 0;
        virtual void resize(uint32_t w, uint32_t h) = 0;
        virtual void waitIdle()                    = 0;

        virtual Vec2 swapchainSize() const         = 0;
        virtual uint32_t currentFrameIndex() const = 0;

        virtual TextureHandle createTexture(const TextureDesc& desc)             = 0;
        virtual void          uploadTexture(TextureHandle tex, const void* data, size_t size) = 0;
        virtual void          destroyTexture(TextureHandle tex)                  = 0;

        virtual BufferHandle  createBuffer(const BufferDesc& desc)               = 0;
        virtual void*         mapBuffer(BufferHandle buf)                        = 0;
        virtual void          unmapBuffer(BufferHandle buf)                      = 0;
        virtual void          destroyBuffer(BufferHandle buf)                    = 0;

        virtual ShaderHandle  createShader(const ShaderDesc& desc)               = 0;
        virtual void          destroyShader(ShaderHandle shader)                 = 0;

        virtual PipelineHandle createPipeline(const PipelineDesc& desc)          = 0;
        virtual void           destroyPipeline(PipelineHandle pipeline)          = 0;

        virtual SamplerHandle  createSampler(const SamplerDesc& desc)            = 0;
        virtual void           destroySampler(SamplerHandle sampler)             = 0;

        virtual void beginRenderPass(const RenderPassDesc& desc)                 = 0;
        virtual void endRenderPass()                                             = 0;

        virtual void setViewport(const Viewport& vp)                             = 0;
        virtual void setScissor(const Rect& rect)                                = 0;
        virtual void setPipeline(PipelineHandle pipeline)                        = 0;

        virtual void setVertexBuffer(BufferHandle buf, uint32_t slot = 0, uint32_t offset = 0)  = 0;
        virtual void setIndexBuffer(BufferHandle buf, IndexType type = IndexType::Uint16)        = 0;
        virtual void setTexture(TextureHandle tex, uint32_t slot)                               = 0;
        virtual void setSampler(SamplerHandle sampler, uint32_t slot)                           = 0;
        virtual void setUniformBuffer(BufferHandle buf, uint32_t slot, uint32_t offset = 0, uint32_t size = 0) = 0;
        virtual void pushConstants(const void* data, uint32_t size, uint32_t offset = 0)       = 0;

        virtual void draw(const DrawCmd& cmd)                                    = 0;
        virtual void drawIndexed(const DrawIndexedCmd& cmd)                      = 0;

        virtual void drawLine(Vec2 a, Vec2 b, Color color, float thickness = 1.0f)                  = 0;
        virtual void drawRect(Rect r, Color color, float thickness = 1.0f)                          = 0;
        virtual void drawFilledRect(Rect r, Color color)                                            = 0;
        virtual void drawCircle(Vec2 center, float radius, Color color, int segments = 32)          = 0;

        virtual Vec2 project(Vec3 worldPos, const Mat4& viewProj) const          = 0;
    };
}
