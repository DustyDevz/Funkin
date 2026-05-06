#include "ui_dx12_text.hpp"

namespace Funkin::Renderer::GAL::UI {
    DX12TextRenderer& DX12TextRenderer::get() {
        static DX12TextRenderer s;
        return s;
    }

    void DX12TextRenderer::init(ID3D12Device* device, ID3D12CommandQueue* queue,
                                IDXGISwapChain3* swapchain, uint32_t frameCount,
                                uint32_t width, uint32_t height) {
        m_width  = width;
        m_height = height;

        UINT d3d11Flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        IUnknown* queues[] = { queue };
        if (FAILED(D3D11On12CreateDevice(device, d3d11Flags, nullptr, 0,
                                        queues, 1, 0,
                                        nullptr, &m_d3d11ctx, nullptr)))
            throw std::runtime_error("Failed to create D3D11On12 device");

        ComPtr<ID3D11Device> d3d11dev;
        m_d3d11ctx->GetDevice(&d3d11dev);
        d3d11dev.As(&m_11on12);

        D2D1_FACTORY_OPTIONS opts{};
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                    __uuidof(ID2D1Factory3), &opts,
                                    reinterpret_cast<void**>(m_d2dFactory.GetAddressOf()))))
            throw std::runtime_error("Failed to create D2D factory");

        ComPtr<IDXGIDevice> dxgiDevice;
        m_11on12.As(&dxgiDevice);
        if (FAILED(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)))
            throw std::runtime_error("Failed to create D2D device");

        m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dCtx);
        m_d2dCtx->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                        __uuidof(IDWriteFactory5),
                                        reinterpret_cast<IUnknown**>(m_dwFactory.GetAddressOf()))))
            throw std::runtime_error("Failed to create DirectWrite factory");

        m_d2dCtx->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 1), &m_brush);

        createBuffers(swapchain, frameCount, width, height);
    }

    void DX12TextRenderer::createBuffers(IDXGISwapChain3* swapchain, uint32_t frameCount,
                                        uint32_t width, uint32_t height) {
        float dpi = 96.0f;

        D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpi, dpi);

        for (uint32_t i = 0; i < frameCount; ++i) {
            ComPtr<ID3D12Resource> backBuffer;
            swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));

            D3D11_RESOURCE_FLAGS flags{ D3D11_BIND_RENDER_TARGET };
            m_11on12->CreateWrappedResource(
                backBuffer.Get(), &flags,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT,
                IID_PPV_ARGS(&m_wrappedBuffers[i]));

            ComPtr<IDXGISurface> surface;
            m_wrappedBuffers[i].As(&surface);
            m_d2dCtx->CreateBitmapFromDxgiSurface(surface.Get(), &bmpProps, &m_d2dTargets[i]);
        }
    }

    void DX12TextRenderer::releaseBuffers() {
        for (auto& b : m_wrappedBuffers) b.Reset();
        for (auto& t : m_d2dTargets)    t.Reset();
    }

    void DX12TextRenderer::resize(IDXGISwapChain3* swapchain, uint32_t frameCount,
                                uint32_t width, uint32_t height) {
        m_width  = width;
        m_height = height;
        releaseBuffers();
        createBuffers(swapchain, frameCount, width, height);
    }

    void DX12TextRenderer::shutdown() {
        releaseBuffers();
        m_formats.clear();
        m_brush.Reset();
        m_d2dCtx.Reset();
        m_d2dDevice.Reset();
        m_d2dFactory.Reset();
        m_dwFactory.Reset();
        m_d3d11ctx.Reset();
        m_11on12.Reset();
    }

void DX12TextRenderer::beginDraw(uint32_t frameIndex) {
    m_11on12->AcquireWrappedResources(m_wrappedBuffers[frameIndex].GetAddressOf(), 1);
    m_d2dCtx->SetTarget(m_d2dTargets[frameIndex].Get());
    m_d2dCtx->BeginDraw();
}

void DX12TextRenderer::endDraw(uint32_t frameIndex) {
    D2D1_TAG tag1 = 0, tag2 = 0;
    HRESULT hr = m_d2dCtx->EndDraw(&tag1, &tag2);
    m_d2dCtx->SetTarget(nullptr);
    m_11on12->ReleaseWrappedResources(m_wrappedBuffers[frameIndex].GetAddressOf(), 1);
    m_d3d11ctx->Flush();
}

    IDWriteTextFormat* DX12TextRenderer::getOrCreateFormat(float fontSize) {
        int key = (int)(fontSize * 10);
        auto it = m_formats.find(key);
        if (it != m_formats.end()) return it->second.Get();

        ComPtr<IDWriteTextFormat> fmt;
        ComPtr<IDWriteFontSetBuilder1> builder;
        ComPtr<IDWriteFontFile> fontFile;

        bool customFont = false;
        if (SUCCEEDED(m_dwFactory->CreateFontSetBuilder(&builder)) &&
            SUCCEEDED(m_dwFactory->CreateFontFileReference(m_fontPath.c_str(), nullptr, &fontFile))) {
            builder->AddFontFile(fontFile.Get());
            ComPtr<IDWriteFontSet> fontSet;
            builder->CreateFontSet(&fontSet);
            ComPtr<IDWriteFontCollection1> collection;
            m_dwFactory->CreateFontCollectionFromFontSet(fontSet.Get(), &collection);

            UINT32 idx = 0; BOOL exists = false;
            collection->FindFamilyName(L"Inter", &idx, &exists);
            if (!exists) { idx = 0; }

            ComPtr<IDWriteFontFamily> family;
            collection->GetFontFamily(idx, &family);
            ComPtr<IDWriteLocalizedStrings> names;
            family->GetFamilyNames(&names);
            wchar_t familyName[64] = {};
            names->GetString(0, familyName, 64);

            if (SUCCEEDED(m_dwFactory->CreateTextFormat(
                    familyName, collection.Get(),
                    DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &fmt)))
                customFont = true;
        }

        if (!customFont) {
            m_dwFactory->CreateTextFormat(
                L"Segoe UI", nullptr,
                DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &fmt);
        }

        fmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        m_formats[key] = fmt;
        return fmt.Get();
    }

    void DX12TextRenderer::drawText(const std::wstring& text, Funkin::Rect bounds,
                                    Funkin::Color color, float fontSize) {
        auto* fmt = getOrCreateFormat(fontSize);
        if (!fmt) return;

        m_brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));

        D2D1_RECT_F r = {
            bounds.x,
            bounds.y,
            bounds.x + bounds.w,
            bounds.y + bounds.h
        };

        m_d2dCtx->DrawText(text.c_str(), (UINT32)text.size(), fmt, r, m_brush.Get());
    }
}