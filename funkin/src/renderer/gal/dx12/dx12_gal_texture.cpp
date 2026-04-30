// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "dx12_gal.hpp"

namespace Funkin::Renderer::GAL {

TextureHandle DX12Gal::createTexture(const TextureDesc& desc)
{
    D3D12_RESOURCE_FLAGS  flags     = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;

    if (desc.usage & TextureUsage::RenderTarget) {
        flags    |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        initState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
    if (desc.usage & TextureUsage::DepthStencil) {
        flags    |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
                    D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        initState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }

    DXGI_FORMAT fmt = toDXGI(desc.format);

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resDesc   = CD3DX12_RESOURCE_DESC::Tex2D(
        fmt,
        static_cast<UINT64>(desc.width),
        static_cast<UINT>(desc.height),
        1,
        static_cast<UINT16>(desc.mips),
        1, 0,
        flags);

    DX12GalTexture tex = {};
    tex.format  = fmt;
    tex.width   = desc.width;
    tex.height  = desc.height;
    tex.mips    = desc.mips;
    tex.usage   = desc.usage;
    tex.state   = initState;

    if (FAILED(m_device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
            initState, nullptr, IID_PPV_ARGS(&tex.resource))))
        throw std::runtime_error("Failed to create texture resource");

    if (desc.usage & TextureUsage::Sampled) {
        tex.srv = m_srvHeap.allocateCPU();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                  = fmt;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels     = desc.mips;
        m_device->CreateShaderResourceView(tex.resource.Get(), &srvDesc, tex.srv);
    }

    if (desc.usage & TextureUsage::RenderTarget) {
        tex.rtv = m_rtvHeap.allocateCPU();
        m_device->CreateRenderTargetView(tex.resource.Get(), nullptr, tex.rtv);
    }

    if (desc.usage & TextureUsage::DepthStencil) {
        tex.dsv = m_dsvHeap.allocateCPU();
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format        = fmt;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags         = D3D12_DSV_FLAG_NONE;
        m_device->CreateDepthStencilView(tex.resource.Get(), &dsvDesc, tex.dsv);
    }

    TextureHandle h;
    h.id = m_textures.insert(std::move(tex));
    return h;
}

void DX12Gal::uploadTexture(TextureHandle texHandle, const void* data, size_t size)
{
    auto* t = m_textures.get(texHandle.id);
    if (!t) return;

    ComPtr<ID3D12CommandAllocator> tmpAlloc;
    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 IID_PPV_ARGS(&tmpAlloc))))
        throw std::runtime_error("Upload: failed to create temp allocator");

    ComPtr<ID3D12GraphicsCommandList> tmpList;
    if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                            tmpAlloc.Get(), nullptr,
                                            IID_PPV_ARGS(&tmpList))))
        throw std::runtime_error("Upload: failed to create temp command list");

    D3D12_RESOURCE_DESC texDesc = t->resource->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
    UINT   numRows        = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes     = 0;
    m_device->GetCopyableFootprints(&texDesc, 0, 1, 0,
                                    &layout, &numRows, &rowSizeInBytes, &totalBytes);

    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufDesc         = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

    ComPtr<ID3D12Resource> uploadBuf;
    if (FAILED(m_device->CreateCommittedResource(
            &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&uploadBuf))))
        throw std::runtime_error("Upload: failed to create staging buffer");

    uint8_t* mapped = nullptr;
    D3D12_RANGE readRange = {};
    uploadBuf->Map(0, &readRange, reinterpret_cast<void**>(&mapped));
    const uint8_t* src = static_cast<const uint8_t*>(data);
    for (UINT row = 0; row < numRows; ++row) {
        memcpy(mapped + static_cast<size_t>(row) * layout.Footprint.RowPitch,
               src    + static_cast<size_t>(row) * static_cast<size_t>(rowSizeInBytes),
               static_cast<size_t>(rowSizeInBytes));
    }
    uploadBuf->Unmap(0, nullptr);

    D3D12_RESOURCE_STATES prevState = t->state;
    D3D12_RESOURCE_BARRIER barrierTo = CD3DX12_RESOURCE_BARRIER::Transition(
        t->resource.Get(), prevState, D3D12_RESOURCE_STATE_COPY_DEST);
    tmpList->ResourceBarrier(1, &barrierTo);

    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource        = t->resource.Get();
    dstLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource       = uploadBuf.Get();
    srcLoc.Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint = layout;

    tmpList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

    D3D12_RESOURCE_STATES afterState = (prevState == D3D12_RESOURCE_STATE_COMMON)
        ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : prevState;

    D3D12_RESOURCE_BARRIER barrierFrom = CD3DX12_RESOURCE_BARRIER::Transition(
        t->resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, afterState);
    tmpList->ResourceBarrier(1, &barrierFrom);
    t->state = afterState;

    tmpList->Close();
    ID3D12CommandList* lists[] = { tmpList.Get() };
    m_queue->ExecuteCommandLists(1, lists);
    waitIdle();
}

void DX12Gal::destroyTexture(TextureHandle tex)
{
    m_textures.remove(tex.id);
}

}
