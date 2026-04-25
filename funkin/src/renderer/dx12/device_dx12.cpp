#include "device_dx12.hpp"
#include <stdexcept>

namespace Funkin::Renderer::DX12 {
    void DX12Device::init() {
        UINT flags = 0;

        #ifdef FUNKIN_DEBUG
            ComPtr<ID3D12Debug> debug;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
                debug->EnableDebugLayer();
                flags = DXGI_CREATE_FACTORY_DEBUG;
            }
        #endif

        if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_factory))))
            throw std::runtime_error("Failed to create DXGI factory");

        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0;
            m_factory->EnumAdapterByGpuPreference(
                i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
            ++i)
        {
            if (SUCCEEDED(D3D12CreateDevice(
                adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                IID_PPV_ARGS(&m_device))))
                break;
        }

        if (!m_device)
            throw std::runtime_error("No DX12 capable GPU found");
    }

    void DX12Device::shutdown() {
        m_device.Reset();
        m_factory.Reset();
    }
}