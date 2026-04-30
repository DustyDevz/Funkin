// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "vk_gal.hpp"

namespace Funkin::Renderer::GAL {

void VKGal::init(const GALDesc&) {}
void VKGal::shutdown() {}
void VKGal::waitIdle() {}

Vec2     VKGal::swapchainSize()     const { return {}; }
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
void VKGal::setScissor(const Rect&)                     { throw std::runtime_error("Vulkan GAL not yet implemented"); }
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
void VKGal::drawLine(Vec2, Vec2, Color, float)          { throw std::runtime_error("Vulkan GAL not yet implemented"); }
void VKGal::drawRect(Rect, Color, float)                { throw std::runtime_error("Vulkan GAL not yet implemented"); }
void VKGal::drawFilledRect(Rect, Color)                 { throw std::runtime_error("Vulkan GAL not yet implemented"); }
void VKGal::drawCircle(Vec2, float, Color, int)         { throw std::runtime_error("Vulkan GAL not yet implemented"); }
Vec2 VKGal::project(Vec3, const Mat4&) const            { throw std::runtime_error("Vulkan GAL not yet implemented"); }

}
