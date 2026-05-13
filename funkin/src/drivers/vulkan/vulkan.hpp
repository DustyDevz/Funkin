// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <drivers/driver_interface.hpp>

namespace Funkin::Renderer::GAL {
    class VKGal : public IDAL {
    public:
        bool     init(const GALDesc& desc)        override;
        void     shutdown()                       override;
        void     beginFrame()                     override;
        void     endFrame()                       override;
        void     resize(uint32_t w, uint32_t h)   override;
        void     waitIdle()                       override;

        Math::Vec2     swapchainSize()     const        override;
        uint32_t currentFrameIndex() const        override;

        TextureHandle  createTexture(const TextureDesc& desc)                                          override;
        void           uploadTexture(TextureHandle tex, const void* data, size_t size)                 override;
        void           destroyTexture(TextureHandle tex)                                               override;

        BufferHandle   createBuffer(const BufferDesc& desc)                                            override;
        void*          mapBuffer(BufferHandle buf)                                                     override;
        void           unmapBuffer(BufferHandle buf)                                                   override;
        void           destroyBuffer(BufferHandle buf)                                                 override;

        ShaderHandle   createShader(const ShaderDesc& desc)                                            override;
        void           destroyShader(ShaderHandle shader)                                              override;

        PipelineHandle createPipeline(const PipelineDesc& desc)                                        override;
        void           destroyPipeline(PipelineHandle pipeline)                                        override;

        SamplerHandle  createSampler(const SamplerDesc& desc)                                          override;
        void           destroySampler(SamplerHandle sampler)                                           override;

        void beginRenderPass(const RenderPassDesc& desc)                                               override;
        void endRenderPass()                                                                           override;
        void setViewport(const Viewport& vp)                                                           override;
        void setScissor(const Math::Rect& rect)                                                              override;
        void setPipeline(PipelineHandle pipeline)                                                      override;
        void setVertexBuffer(BufferHandle buf, uint32_t slot = 0, uint32_t offset = 0)                 override;
        void setIndexBuffer(BufferHandle buf, IndexType type = IndexType::Uint16)                      override;
        void setTexture(TextureHandle tex, uint32_t slot)                                              override;
        void setSampler(SamplerHandle sampler, uint32_t slot)                                          override;
        void setUniformBuffer(BufferHandle buf, uint32_t slot,
                            uint32_t offset = 0, uint32_t size = 0)                                  override;
        void pushConstants(const void* data, uint32_t size, uint32_t offset = 0)                       override;
        void draw(const DrawCmd& cmd)                                                                  override;
        void drawIndexed(const DrawIndexedCmd& cmd)                                                    override;
        void drawLine(Math::Vec2 a, Math::Vec2 b, Math::Color color, float thickness = 1.0f)                             override;
        void drawRect(Math::Rect r, Math::Color color, float thickness = 1.0f)                                     override;
        void drawFilledRect(Math::Rect r, Math::Color color)                                                       override;
        void drawCircle(Math::Vec2 center, float radius, Math::Color color, int segments = 32)                     override;
        Math::Vec2 project(Math::Vec3 worldPos, const Math::Mat4& viewProj) const                                        override;
    };
}
