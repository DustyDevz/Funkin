// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "vulkan.hpp"

namespace Funkin::Renderer::GAL {
    bool VKGal::init(const GALDesc&) {return true;}
    void VKGal::shutdown() {}
    void VKGal::waitIdle() {}

    Math::Vec2     VKGal::swapchainSize()     const { return {}; }
    uint32_t VKGal::currentFrameIndex() const { return 0; }

    void VKGal::beginFrame()                        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::endFrame()                          { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::resize(uint32_t, uint32_t)          { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    TextureHandle VKGal::createTexture(const TextureDesc&)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::uploadTexture(TextureHandle, const void*, size_t)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::destroyTexture(TextureHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    BufferHandle VKGal::createBuffer(const BufferDesc&)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void* VKGal::mapBuffer(BufferHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::unmapBuffer(BufferHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::destroyBuffer(BufferHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    ShaderHandle VKGal::createShader(const ShaderDesc&)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::destroyShader(ShaderHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    PipelineHandle VKGal::createPipeline(const PipelineDesc&)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::destroyPipeline(PipelineHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    SamplerHandle VKGal::createSampler(const SamplerDesc&)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::destroySampler(SamplerHandle)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }

    void VKGal::beginRenderPass(const RenderPassDesc&)      { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::endRenderPass()                             { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setViewport(const Viewport&)                { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setScissor(const Math::Rect&)                     { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setPipeline(PipelineHandle)                 { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setVertexBuffer(BufferHandle, uint32_t, uint32_t)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setIndexBuffer(BufferHandle, IndexType)     { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setTexture(TextureHandle, uint32_t)         { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setSampler(SamplerHandle, uint32_t)         { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::setUniformBuffer(BufferHandle, uint32_t, uint32_t, uint32_t)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::pushConstants(const void*, uint32_t, uint32_t)
        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::draw(const DrawCmd&)                        { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::drawIndexed(const DrawIndexedCmd&)          { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::drawLine(Math::Vec2, Math::Vec2, Math::Color, float)          { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::drawRect(Math::Rect, Math::Color, float)                { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::drawFilledRect(Math::Rect, Math::Color)                 { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    void VKGal::drawCircle(Math::Vec2, float, Math::Color, int)         { throw std::runtime_error("Vulkan GAL not yet implemented"); }
    Math::Vec2 VKGal::project(Math::Vec3, const Math::Mat4&) const            { throw std::runtime_error("Vulkan GAL not yet implemented"); }
}
