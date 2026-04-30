// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Renderer::DX12 {
    class DX12Device {
    public:
        void init();
        void shutdown();

        ID3D12Device*  device()  const { return m_device.Get();  }
        IDXGIFactory6* factory() const { return m_factory.Get(); }

    private:
        ComPtr<ID3D12Device>  m_device;
        ComPtr<IDXGIFactory6> m_factory;
    };
}
