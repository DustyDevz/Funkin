// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "dx12_gal.hpp"

namespace Funkin::Renderer::GAL {
    void DX12Gal::init(const GALDesc& desc) {
        m_width  = desc.width;
        m_height = desc.height;
        m_vsync  = desc.vsync;

        UINT dxgiFlags = 0;
        #ifdef FUNKIN_DEBUG
            {
                ComPtr<ID3D12Debug> debugCtrl;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugCtrl)))) {
                    debugCtrl->EnableDebugLayer();
                    dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }
            }
        #endif

        if (FAILED(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&m_factory))))
            throw std::runtime_error("Failed to create DXGI factory");

        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0;
            SUCCEEDED(m_factory->EnumAdapterByGpuPreference(
                i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
            ++i)
        {
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                                            IID_PPV_ARGS(&m_device))))
                break;
            adapter.Reset();
        }

        if (!m_device) {
            m_factory->EnumAdapters1(0, &adapter);
            if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                                        IID_PPV_ARGS(&m_device))))
                throw std::runtime_error("Failed to create D3D12 device");
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue))))
            throw std::runtime_error("Failed to create command queue");

        HWND hwnd = static_cast<HWND>(desc.windowHandle);

        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width       = desc.width;
        scDesc.Height      = desc.height;
        scDesc.Format      = toDXGI(desc.swapFormat);
        scDesc.Stereo      = FALSE;
        scDesc.SampleDesc  = { 1, 0 };
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount = desc.frameCount;
        scDesc.Scaling     = DXGI_SCALING_STRETCH;
        scDesc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
        scDesc.Flags       = 0;

        ComPtr<IDXGISwapChain1> sc1;
        if (FAILED(m_factory->CreateSwapChainForHwnd(m_queue.Get(), hwnd, &scDesc,
                                                    nullptr, nullptr, &sc1)))
            throw std::runtime_error("Failed to create swap chain");

        m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        sc1.As(&m_swapchain);

        m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

        for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
            if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                        IID_PPV_ARGS(&m_alloc[i]))))
                throw std::runtime_error("Failed to create command allocator");
        }

        if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  m_alloc[0].Get(), nullptr,
                                                  IID_PPV_ARGS(&m_cmdList))))
            throw std::runtime_error("Failed to create command list");
        m_cmdList->Close();

        if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
            throw std::runtime_error("Failed to create fence");

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent)
            throw std::runtime_error("Failed to create fence event");

        m_fenceValue = 0;
        for (uint32_t i = 0; i < FRAME_COUNT; ++i)
            m_frameFenceValues[i] = 0;

        m_rtvHeap.init(m_device.Get(),     D3D12_DESCRIPTOR_HEAP_TYPE_RTV,         64,   false);
        m_dsvHeap.init(m_device.Get(),     D3D12_DESCRIPTOR_HEAP_TYPE_DSV,         16,   false);
        m_srvHeap.init(m_device.Get(),     D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true);
        m_samplerHeap.init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,     64,   true);

        for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
            if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapTargets[i]))))
                throw std::runtime_error("Failed to get swap chain buffer");
            m_swapRTVs[i] = m_rtvHeap.allocateCPU();
            m_device->CreateRenderTargetView(m_swapTargets[i].Get(), nullptr, m_swapRTVs[i]);
            m_swapTargetStates[i] = D3D12_RESOURCE_STATE_PRESENT;
        }
    }

    void DX12Gal::shutdown() {
        waitIdle();

        m_textures.slots.clear();
        m_buffers.slots.clear();
        m_shaders.slots.clear();
        m_pipelines.slots.clear();
        m_samplers.slots.clear();

        m_rtvHeap.shutdown();
        m_dsvHeap.shutdown();
        m_srvHeap.shutdown();
        m_samplerHeap.shutdown();

        for (uint32_t i = 0; i < FRAME_COUNT; ++i)
            m_swapTargets[i].Reset();

        m_cmdList.Reset();

        for (uint32_t i = 0; i < FRAME_COUNT; ++i)
            m_alloc[i].Reset();

        m_swapchain.Reset();
        m_queue.Reset();

        if (m_fenceEvent) {
            CloseHandle(m_fenceEvent);
            m_fenceEvent = nullptr;
        }
        m_fence.Reset();
        m_factory.Reset();
        m_device.Reset();
    }

    void DX12Gal::beginFrame() {
        waitForFrame(m_frameIndex);
        m_alloc[m_frameIndex]->Reset();
        m_cmdList->Reset(m_alloc[m_frameIndex].Get(), nullptr);

        ID3D12DescriptorHeap* heaps[] = { m_srvHeap.heap(), m_samplerHeap.heap() };
        m_cmdList->SetDescriptorHeaps(2, heaps);
    }

    void DX12Gal::endFrame() {
        if (m_swapTargetStates[m_frameIndex] == D3D12_RESOURCE_STATE_RENDER_TARGET) {
            transitionBarrier(m_swapTargets[m_frameIndex].Get(),
                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                            D3D12_RESOURCE_STATE_PRESENT);
            m_swapTargetStates[m_frameIndex] = D3D12_RESOURCE_STATE_PRESENT;
        }

        m_cmdList->Close();

        ID3D12CommandList* lists[] = { m_cmdList.Get() };
        m_queue->ExecuteCommandLists(1, lists);

        signalFrame(m_frameIndex);

        m_swapchain->Present(m_vsync ? 1 : 0, 0);
        m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
    }

    void DX12Gal::resize(uint32_t w, uint32_t h) {
        waitIdle();

        for (uint32_t i = 0; i < FRAME_COUNT; ++i)
            m_swapTargets[i].Reset();

        m_swapchain->ResizeBuffers(FRAME_COUNT, w, h, DXGI_FORMAT_UNKNOWN, 0);

        for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
            m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapTargets[i]));
            m_device->CreateRenderTargetView(m_swapTargets[i].Get(), nullptr, m_swapRTVs[i]);
            m_swapTargetStates[i] = D3D12_RESOURCE_STATE_PRESENT;
        }

        m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
        m_width  = w;
        m_height = h;
    }

    void DX12Gal::waitIdle() {
        UINT64 val = ++m_fenceValue;
        m_queue->Signal(m_fence.Get(), val);
        if (m_fence->GetCompletedValue() < val) {
            m_fence->SetEventOnCompletion(val, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    Vec2 DX12Gal::swapchainSize() const {
        return { static_cast<float>(m_width), static_cast<float>(m_height) };
    }

    uint32_t DX12Gal::currentFrameIndex() const {
        return m_frameIndex;
    }

    void DX12Gal::waitForFrame(uint32_t frame) {
        if (m_fence->GetCompletedValue() < m_frameFenceValues[frame]) {
            m_fence->SetEventOnCompletion(m_frameFenceValues[frame], m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void DX12Gal::signalFrame(uint32_t frame)  {
        m_frameFenceValues[frame] = ++m_fenceValue;
        m_queue->Signal(m_fence.Get(), m_fenceValue);
    }

    void DX12Gal::transitionBarrier(ID3D12Resource* resource,
                                    D3D12_RESOURCE_STATES before,
                                    D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);
        m_cmdList->ResourceBarrier(1, &b);
    }

    DXGI_FORMAT DX12Gal::toDXGI(PixelFormat fmt) {
        switch (fmt) {
            case PixelFormat::RGBA8_Unorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
            case PixelFormat::RGBA8_Srgb:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case PixelFormat::BGRA8_Unorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
            case PixelFormat::BGRA8_Srgb:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case PixelFormat::RGBA16_Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case PixelFormat::RG16_Float:        return DXGI_FORMAT_R16G16_FLOAT;
            case PixelFormat::R16_Float:         return DXGI_FORMAT_R16_FLOAT;
            case PixelFormat::R32_Float:         return DXGI_FORMAT_R32_FLOAT;
            case PixelFormat::RG32_Float:        return DXGI_FORMAT_R32G32_FLOAT;
            case PixelFormat::RGB32_Float:       return DXGI_FORMAT_R32G32B32_FLOAT;
            case PixelFormat::RGBA32_Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case PixelFormat::D32_Float:         return DXGI_FORMAT_D32_FLOAT;
            case PixelFormat::D24_Unorm_S8_Uint: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case PixelFormat::D16_Unorm:         return DXGI_FORMAT_D16_UNORM;
            default:                             return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D12_RESOURCE_STATES DX12Gal::toD3D12State(TextureUsage usage) {
        if (usage & TextureUsage::DepthStencil) return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (usage & TextureUsage::RenderTarget) return D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (usage & TextureUsage::Sampled)      return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        return D3D12_RESOURCE_STATE_COMMON;
    }
}
