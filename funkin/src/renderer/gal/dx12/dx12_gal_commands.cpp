// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "dx12_gal.hpp"
#include <assert.h>

namespace Funkin::Renderer::GAL {
    void DX12Gal::beginRenderPass(const RenderPassDesc& desc) {
        m_renderPassIsSwapchain = desc.useSwapchainTarget;
        m_renderPassColors.clear();
        m_renderPassHasDepth = false;

        if (desc.useSwapchainTarget) {
            if (m_swapTargetStates[m_frameIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET) {
                transitionBarrier(m_swapTargets[m_frameIndex].Get(),
                                m_swapTargetStates[m_frameIndex],
                                D3D12_RESOURCE_STATE_RENDER_TARGET);
                m_swapTargetStates[m_frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE& rtv = m_swapRTVs[m_frameIndex];

            if (desc.colorAttachments && desc.colorCount > 0 &&
                desc.colorAttachments[0].loadOp == LoadOp::Clear)
            {
                const Color& cc = desc.colorAttachments[0].clearColor;
                FLOAT clearColor[4] = { cc.r, cc.g, cc.b, cc.a };
                m_cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
            }

            m_cmdList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

            D3D12_VIEWPORT vp = {};
            vp.Width    = static_cast<float>(m_width);
            vp.Height   = static_cast<float>(m_height);
            vp.MaxDepth = 1.0f;
            m_cmdList->RSSetViewports(1, &vp);

            D3D12_RECT scissor = { 0, 0,
                                static_cast<LONG>(m_width),
                                static_cast<LONG>(m_height) };
            m_cmdList->RSSetScissorRects(1, &scissor);
        } else {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle     = {};
            bool     hasDsv = false;
            uint32_t rw = 0, rh = 0;

            for (uint32_t i = 0; i < desc.colorCount; ++i) {
                auto* t = m_textures.get(desc.colorAttachments[i].texture.id);
                if (!t) continue;

                if (t->state != D3D12_RESOURCE_STATE_RENDER_TARGET) {
                    transitionBarrier(t->resource.Get(), t->state,
                                    D3D12_RESOURCE_STATE_RENDER_TARGET);
                    t->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
                }

                rtvHandles[i] = t->rtv;

                if (desc.colorAttachments[i].loadOp == LoadOp::Clear) {
                    const Color& cc = desc.colorAttachments[i].clearColor;
                    FLOAT clearColor[4] = { cc.r, cc.g, cc.b, cc.a };
                    m_cmdList->ClearRenderTargetView(t->rtv, clearColor, 0, nullptr);
                }

                m_renderPassColors.push_back(desc.colorAttachments[i].texture);
                rw = t->width;
                rh = t->height;
            }

            if (desc.depthAttachment) {
                auto* dt = m_textures.get(desc.depthAttachment->texture.id);
                if (dt) {
                    if (dt->state != D3D12_RESOURCE_STATE_DEPTH_WRITE) {
                        transitionBarrier(dt->resource.Get(), dt->state,
                                        D3D12_RESOURCE_STATE_DEPTH_WRITE);
                        dt->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                    }

                    dsvHandle = dt->dsv;
                    hasDsv    = true;
                    m_renderPassHasDepth = true;
                    m_renderPassDepth    = desc.depthAttachment->texture;

                    if (desc.depthAttachment->loadOp == LoadOp::Clear) {
                        m_cmdList->ClearDepthStencilView(
                            dt->dsv,
                            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                            desc.depthAttachment->clearDepth,
                            desc.depthAttachment->clearStencil,
                            0, nullptr);
                    }
                }
            }

            m_cmdList->OMSetRenderTargets(desc.colorCount, rtvHandles, FALSE,
                                          hasDsv ? &dsvHandle : nullptr);

            D3D12_VIEWPORT vp = {};
            vp.Width    = static_cast<float>(rw);
            vp.Height   = static_cast<float>(rh);
            vp.MaxDepth = 1.0f;
            m_cmdList->RSSetViewports(1, &vp);

            D3D12_RECT scissor = { 0, 0,
                                static_cast<LONG>(rw),
                                static_cast<LONG>(rh) };
            m_cmdList->RSSetScissorRects(1, &scissor);
        }

        m_inRenderPass = true;
    }

    void DX12Gal::endRenderPass() {
        if (!m_renderPassIsSwapchain) {
            for (const TextureHandle& colorHandle : m_renderPassColors) {
                auto* t = m_textures.get(colorHandle.id);
                if (!t) continue;
                if (t->state != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
                    transitionBarrier(t->resource.Get(), t->state,
                                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    t->state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                }
            }
        }

        m_renderPassColors.clear();
        m_renderPassHasDepth    = false;
        m_renderPassIsSwapchain = false;
        m_inRenderPass          = false;
    }

    void DX12Gal::setViewport(const Viewport& vp) {
        D3D12_VIEWPORT d3dVp = {};
        d3dVp.TopLeftX = vp.x;
        d3dVp.TopLeftY = vp.y;
        d3dVp.Width    = vp.width;
        d3dVp.Height   = vp.height;
        d3dVp.MinDepth = vp.minZ;
        d3dVp.MaxDepth = vp.maxZ;
        m_cmdList->RSSetViewports(1, &d3dVp);
    }

    void DX12Gal::setScissor(const Rect& rect) {
        D3D12_RECT scissor = {
            static_cast<LONG>(rect.x),
            static_cast<LONG>(rect.y),
            static_cast<LONG>(rect.x + rect.w),
            static_cast<LONG>(rect.y + rect.h)
        };
        m_cmdList->RSSetScissorRects(1, &scissor);
    }

    void DX12Gal::setPipeline(PipelineHandle pipeline) {
        auto* p = m_pipelines.get(pipeline.id);
        if (!p) return;
        m_cmdList->SetGraphicsRootSignature(p->rootSig.Get());
        m_cmdList->SetPipelineState(p->pso.Get());
        m_currentTopology     = p->topology;
        m_currentVertexStride = p->vertexStride;
    }

    void DX12Gal::setVertexBuffer(BufferHandle buf, uint32_t slot, uint32_t offset) {
        auto* b = m_buffers.get(buf.id);
        if (!b) return;

        UINT safeSize = (offset < b->size) ? static_cast<UINT>(b->size - offset) : 0u;

        D3D12_VERTEX_BUFFER_VIEW vbv = {};
        vbv.BufferLocation = b->resource->GetGPUVirtualAddress() + offset;
        vbv.SizeInBytes    = safeSize;
        vbv.StrideInBytes  = m_currentVertexStride;
        m_cmdList->IASetVertexBuffers(slot, 1, &vbv);
    }

    void DX12Gal::setIndexBuffer(BufferHandle buf, IndexType type) {
        auto* b = m_buffers.get(buf.id);
        if (!b) return;

        D3D12_INDEX_BUFFER_VIEW ibv = {};
        ibv.BufferLocation = b->resource->GetGPUVirtualAddress();
        ibv.SizeInBytes    = static_cast<UINT>(b->size);
        ibv.Format         = (type == IndexType::Uint32) ? DXGI_FORMAT_R32_UINT
                                                        : DXGI_FORMAT_R16_UINT;
        m_cmdList->IASetIndexBuffer(&ibv);
    }

    void DX12Gal::setTexture(TextureHandle tex, uint32_t slot) {
        auto* t = m_textures.get(tex.id);
        if (!t || slot >= kSrvDynSlots) return;

        uint32_t dynBase = kSrvStaticCap + m_frameIndex * kSrvDynSlots * kMaxDrawsPerFrame
                        + m_drawCallIndex * kSrvDynSlots;

        assert(dynBase + slot < 4096 && "SRV heap overflow");

        D3D12_CPU_DESCRIPTOR_HANDLE dst = m_srvHeap.cpuAt(dynBase + slot);
        m_device->CopyDescriptorsSimple(1, dst, t->srv,
                                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_cmdList->SetGraphicsRootDescriptorTable(0, m_srvHeap.gpuAt(dynBase));
    }

    void DX12Gal::setSampler(SamplerHandle sampler, uint32_t slot) {
        auto* s = m_samplers.get(sampler.id);
        if (!s || slot >= kSamplerDynSlots) return;

        uint32_t dynBase = kSamplerStaticCap + m_frameIndex * kSamplerDynSlots * kMaxDrawsPerFrame
                        + m_drawCallIndex * kSamplerDynSlots;

        assert(dynBase + slot < 2048 && "Sampler heap overflow");

        D3D12_CPU_DESCRIPTOR_HANDLE dst = m_samplerHeap.cpuAt(dynBase + slot);
        m_device->CopyDescriptorsSimple(1, dst, s->handle,
                                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        m_cmdList->SetGraphicsRootDescriptorTable(1, m_samplerHeap.gpuAt(dynBase));
    }

    void DX12Gal::setUniformBuffer(BufferHandle buf, uint32_t slot,
                                    uint32_t offset, uint32_t size)
    {
        auto* b = m_buffers.get(buf.id);
        if (!b || slot >= kCbvDynSlots) return;

        uint32_t cbvDynBase = kSrvStaticCap
                            + FRAME_COUNT * kSrvDynSlots * kMaxDrawsPerFrame
                            + m_frameIndex * kCbvDynSlots * kMaxDrawsPerFrame
                            + m_drawCallIndex * kCbvDynSlots;

        UINT cbSize = (size == 0) ? static_cast<UINT>(b->size) : size;
        cbSize = (cbSize + 255u) & ~255u;

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = b->resource->GetGPUVirtualAddress() + offset;
        cbvDesc.SizeInBytes    = cbSize;

        D3D12_CPU_DESCRIPTOR_HANDLE dst = m_srvHeap.cpuAt(cbvDynBase + slot);
        m_device->CreateConstantBufferView(&cbvDesc, dst);
        m_cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.gpuAt(cbvDynBase));
    }

    void DX12Gal::pushConstants(const void* data, uint32_t size, uint32_t offset) {
        uint32_t num32 = (size + 3u) / 4u;
        uint32_t off32 = offset / 4u;
        m_cmdList->SetGraphicsRoot32BitConstants(3, num32, data, off32);
    }

    void DX12Gal::draw(const DrawCmd& cmd) {
        m_cmdList->IASetPrimitiveTopology(m_currentTopology);
        m_cmdList->DrawInstanced(cmd.vertexCount, cmd.instanceCount,
                                cmd.firstVertex, cmd.firstInstance);
    }

    void DX12Gal::drawIndexed(const DrawIndexedCmd& cmd) {
        m_cmdList->IASetPrimitiveTopology(m_currentTopology);
        m_cmdList->DrawIndexedInstanced(cmd.indexCount, cmd.instanceCount,
                                        cmd.firstIndex, cmd.vertexOffset,
                                        cmd.firstInstance);
        ++m_drawCallIndex;
    }

    void DX12Gal::drawLine(Vec2, Vec2, Color, float)        {}
    void DX12Gal::drawRect(Rect, Color, float)              {}
    void DX12Gal::drawFilledRect(Rect, Color)               {}
    void DX12Gal::drawCircle(Vec2, float, Color, int)       {}

    Vec2 DX12Gal::project(Vec3 worldPos, const Mat4& viewProj) const {
        Vec4 clip = viewProj * Vec4(worldPos, 1.0f);
        Vec3 ndc  = clip.xyz() / clip.w;
        return {
            (ndc.x * 0.5f + 0.5f) * static_cast<float>(m_width),
            (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(m_height)
        };
    }
}
