// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "renderer/gal/idal.hpp"
#include "renderer/renderer_common.hpp"
#include "dx12_gal_internal.hpp"
#include "dx12_gal_descriptors.hpp"

namespace Funkin::Renderer::GAL {

class DX12Gal : public IDAL {
    public:
        void     init(const GALDesc& desc)        override;
        void     shutdown()                       override;
        void     beginFrame()                     override;
        void     endFrame()                       override;
        void     resize(uint32_t w, uint32_t h)   override;
        void     waitIdle()                       override;

        Vec2     swapchainSize()     const        override;
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
        void setScissor(const Rect& rect)                                                              override;
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
        void drawLine(Vec2 a, Vec2 b, Color color, float thickness = 1.0f)                             override;
        void drawRect(Rect r, Color color, float thickness = 1.0f)                                     override;
        void drawFilledRect(Rect r, Color color)                                                       override;
        void drawCircle(Vec2 center, float radius, Color color, int segments = 32)                     override;
        Vec2 project(Vec3 worldPos, const Mat4& viewProj) const                                        override;

    private:
        void waitForFrame(uint32_t frame);
        void signalFrame(uint32_t frame);
        void transitionBarrier(ID3D12Resource* resource,
                            D3D12_RESOURCE_STATES before,
                            D3D12_RESOURCE_STATES after);
        DXGI_FORMAT                 toDXGI(PixelFormat fmt);
        D3D12_RESOURCE_STATES       toD3D12State(TextureUsage usage);
        ComPtr<ID3D12RootSignature> buildRootSignature(const PipelineDesc& desc);

        ComPtr<ID3D12Device>              m_device;
        ComPtr<IDXGIFactory6>             m_factory;
        ComPtr<IDXGISwapChain3>           m_swapchain;
        ComPtr<ID3D12CommandQueue>        m_queue;
        ComPtr<ID3D12CommandAllocator>    m_alloc[FRAME_COUNT];
        ComPtr<ID3D12GraphicsCommandList> m_cmdList;
        ComPtr<ID3D12Fence>               m_fence;
        HANDLE                            m_fenceEvent             = nullptr;
        UINT64                            m_fenceValue             = 0;
        UINT64                            m_frameFenceValues[FRAME_COUNT] = {};
        ComPtr<ID3D12Resource>            m_swapTargets[FRAME_COUNT];
        D3D12_RESOURCE_STATES             m_swapTargetStates[FRAME_COUNT] = {};

        DX12DescriptorHeap           m_rtvHeap;
        DX12DescriptorHeap           m_dsvHeap;
        DX12DescriptorHeap           m_srvHeap;
        DX12DescriptorHeap           m_samplerHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE  m_swapRTVs[FRAME_COUNT] = {};

        ResourcePool<DX12GalTexture>  m_textures;
        ResourcePool<DX12GalBuffer>   m_buffers;
        ResourcePool<DX12GalShader>   m_shaders;
        ResourcePool<DX12GalPipeline> m_pipelines;
        ResourcePool<DX12GalSampler>  m_samplers;

        uint32_t m_frameIndex   = 0;
        uint32_t m_width        = 0;
        uint32_t m_height       = 0;
        bool     m_vsync        = false;
        bool     m_inRenderPass = false;

        D3D12_PRIMITIVE_TOPOLOGY m_currentTopology     = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        uint32_t                 m_currentVertexStride  = 0;

        bool                       m_renderPassIsSwapchain = false;
        std::vector<TextureHandle> m_renderPassColors;
        bool                       m_renderPassHasDepth    = false;
        TextureHandle              m_renderPassDepth       = {};

        static constexpr uint32_t kSrvStaticCap     = 512;
        static constexpr uint32_t kSrvDynSlots      = 8;
        static constexpr uint32_t kCbvDynSlots      = 4;
        static constexpr uint32_t kSamplerStaticCap = 32;
        static constexpr uint32_t kSamplerDynSlots  = 8;
    };
}
