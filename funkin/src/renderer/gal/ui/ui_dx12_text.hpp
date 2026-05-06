// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Renderer::GAL::UI {
    struct DX12TextFormat {
        ComPtr<IDWriteTextFormat> format;
        float                     size = 0.0f;
    };

    class DX12TextRenderer {
    public:
        static DX12TextRenderer& get();

        void init(ID3D12Device* device, ID3D12CommandQueue* queue,
                IDXGISwapChain3* swapchain, uint32_t frameCount,
                uint32_t width, uint32_t height);
        void shutdown();
        void resize(IDXGISwapChain3* swapchain, uint32_t frameCount,
                    uint32_t width, uint32_t height);

        void beginDraw(uint32_t frameIndex);
        void endDraw(uint32_t frameIndex);

        void drawText(const std::wstring& text, Funkin::Rect bounds,
                    Funkin::Color color, float fontSize);

        void setFontFile(const std::wstring& path) { m_fontPath = path; }

    private:
        DX12TextRenderer() = default;

        IDWriteTextFormat* getOrCreateFormat(float fontSize);
        void createBuffers(IDXGISwapChain3* swapchain, uint32_t frameCount,
                        uint32_t width, uint32_t height);
        void releaseBuffers();

        ComPtr<ID3D11On12Device>        m_11on12;
        ComPtr<ID3D11DeviceContext>     m_d3d11ctx;
        ComPtr<ID2D1Factory3>           m_d2dFactory;
        ComPtr<ID2D1Device2>            m_d2dDevice;
        ComPtr<ID2D1DeviceContext2>     m_d2dCtx;
        ComPtr<IDWriteFactory5>         m_dwFactory;
        ComPtr<ID2D1SolidColorBrush>    m_brush;

        static constexpr uint32_t MAX_FRAMES = 3;
        ComPtr<ID3D11Resource>          m_wrappedBuffers[MAX_FRAMES];
        ComPtr<ID2D1Bitmap1>            m_d2dTargets[MAX_FRAMES];

        std::unordered_map<int, ComPtr<IDWriteTextFormat>> m_formats;
        std::wstring m_fontPath = L"fonts/reg.ttf";

        uint32_t m_width  = 0;
        uint32_t m_height = 0;

        ComPtr<IDWriteFontCollection1> m_fontCollection;
        wchar_t m_familyName[64] = {};
    };
}