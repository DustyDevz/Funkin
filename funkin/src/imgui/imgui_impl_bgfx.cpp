// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <bx/math.h>
#include "imgui_impl_bgfx.hpp"
#include "shaders/imgui/vs_ocornut_imgui.bin.h"
#include "shaders/imgui/fs_ocornut_imgui.bin.h"

struct BGFXData {
    bgfx::ViewId            viewId      = 255;
    bgfx::TextureHandle     fontTex     = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle     uTexture    = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle     program     = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout      layout;
};

static BGFXData* s_data = nullptr;

namespace Funkin::ImGui_BGFX {
    bool init(bgfx::ViewId viewId) {
        s_data         = new BGFXData();
        s_data->viewId = viewId;

        s_data->layout
            .begin()
            .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
        .end();

        const bgfx::Memory* vsMem = nullptr;
        const bgfx::Memory* fsMem = nullptr;
        
        switch (bgfx::getRendererType()) {
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12:
                vsMem = bgfx::makeRef(vs_ocornut_imgui_dxbc, sizeof(vs_ocornut_imgui_dxbc));
                fsMem = bgfx::makeRef(fs_ocornut_imgui_dxbc, sizeof(fs_ocornut_imgui_dxbc));
                break;
            case bgfx::RendererType::Vulkan:
                vsMem = bgfx::makeRef(vs_ocornut_imgui_spv, sizeof(vs_ocornut_imgui_spv));
                fsMem = bgfx::makeRef(fs_ocornut_imgui_spv, sizeof(fs_ocornut_imgui_spv));
                break;
            case bgfx::RendererType::Metal:
                vsMem = bgfx::makeRef(vs_ocornut_imgui_mtl, sizeof(vs_ocornut_imgui_mtl));
                fsMem = bgfx::makeRef(fs_ocornut_imgui_mtl, sizeof(fs_ocornut_imgui_mtl));
                break;
            case bgfx::RendererType::OpenGL:
                vsMem = bgfx::makeRef(vs_ocornut_imgui_glsl, sizeof(vs_ocornut_imgui_glsl));
                fsMem = bgfx::makeRef(fs_ocornut_imgui_glsl, sizeof(fs_ocornut_imgui_glsl));
                break;
            default:
                vsMem = bgfx::makeRef(vs_ocornut_imgui_glsl, sizeof(vs_ocornut_imgui_glsl));
                fsMem = bgfx::makeRef(fs_ocornut_imgui_glsl, sizeof(fs_ocornut_imgui_glsl));
                break;
        }

        bgfx::ShaderHandle vs = bgfx::createShader(vsMem);
        bgfx::ShaderHandle fs = bgfx::createShader(fsMem);
        s_data->program       = bgfx::createProgram(vs, fs, true);

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_bgfx";
        io.BackendFlags       |= ImGuiBackendFlags_RendererHasVtxOffset;

        uint8_t* pixels;
        int fw, fh;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &fw, &fh);

        s_data->fontTex = bgfx::createTexture2D(
            (uint16_t)fw, (uint16_t)fh, false, 1,
            bgfx::TextureFormat::BGRA8,
            0, bgfx::copy(pixels, fw * fh * 4)
        );

        io.Fonts->SetTexID((ImTextureID)(uintptr_t)s_data->fontTex.idx);
        s_data->uTexture = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        return true;
    }

    void shutdown() {
        bgfx::destroy(s_data->fontTex);
        bgfx::destroy(s_data->uTexture);
        bgfx::destroy(s_data->program);
        delete s_data;
        s_data = nullptr;
    }

    void newFrame() {}

    void render(ImDrawData* drawData) {
        if (!drawData || drawData->CmdListsCount == 0) return;

        const float L = drawData->DisplayPos.x;
        const float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        const float T = drawData->DisplayPos.y;
        const float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

        float ortho[16];
        bx::mtxOrtho(ortho, L, R, B, T, 0.0f, 1.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(s_data->viewId, nullptr, ortho);
        bgfx::setViewRect(s_data->viewId, 0, 0,
            (uint16_t)drawData->DisplaySize.x,
            (uint16_t)drawData->DisplaySize.y);

        for (int n = 0; n < drawData->CmdListsCount; ++n) {
            const ImDrawList* cmdList = drawData->CmdLists[n];

            bgfx::TransientVertexBuffer tvb;
            bgfx::TransientIndexBuffer  tib;

            uint32_t numVerts = (uint32_t)cmdList->VtxBuffer.Size;
            uint32_t numIdx   = (uint32_t)cmdList->IdxBuffer.Size;

            if (!bgfx::getAvailTransientVertexBuffer(numVerts, s_data->layout) ||
                !bgfx::getAvailTransientIndexBuffer(numIdx))
                break;

            bgfx::allocTransientVertexBuffer(&tvb, numVerts, s_data->layout);
            bgfx::allocTransientIndexBuffer(&tib, numIdx);

            memcpy(tvb.data, cmdList->VtxBuffer.Data, numVerts * sizeof(ImDrawVert));
            memcpy(tib.data, cmdList->IdxBuffer.Data, numIdx   * sizeof(ImDrawIdx));

            for (const ImDrawCmd& cmd : cmdList->CmdBuffer) {
                if (cmd.UserCallback) {
                    cmd.UserCallback(cmdList, &cmd);
                    continue;
                }

                const uint16_t sx = (uint16_t)bx::max(cmd.ClipRect.x - drawData->DisplayPos.x, 0.0f);
                const uint16_t sy = (uint16_t)bx::max(cmd.ClipRect.y - drawData->DisplayPos.y, 0.0f);
                const uint16_t sw = (uint16_t)bx::min(cmd.ClipRect.z - drawData->DisplayPos.x, 65535.0f) - sx;
                const uint16_t sh = (uint16_t)bx::min(cmd.ClipRect.w - drawData->DisplayPos.y, 65535.0f) - sy;
                bgfx::setScissor(sx, sy, sw, sh);

                bgfx::TextureHandle th = { (uint16_t)(uintptr_t)cmd.GetTexID() };
                bgfx::setTexture(0, s_data->uTexture, th);
                bgfx::setVertexBuffer(0, &tvb, cmd.VtxOffset, numVerts);
                bgfx::setIndexBuffer(&tib, cmd.IdxOffset, cmd.ElemCount);
                bgfx::setState(
                    BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                    BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                );
                bgfx::submit(s_data->viewId, s_data->program);
            }
        }
    }
}