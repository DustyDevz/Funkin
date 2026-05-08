// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "core/engine.hpp"
#include "core/log.hpp"
#include "renderer/shader/shader_loader.hpp"
#include "renderer/shader/shader_types.hpp"
#include "scene/components/component_test.hpp"
#include "ui/ui_renderer.hpp"
#include <input/input.hpp>
#include <platform/input/input_win32.hpp>
#include <ui/screens/screen_project.hpp>
#include <math/vec2.hpp>
#include <renderer/gal/ui/ui_dx12_text.hpp>
#include <renderer/gal/dx12/dx12_gal.hpp>

namespace GAL    = Funkin::Renderer::GAL;
namespace Shader = Funkin::Renderer::Shader;

struct AppState {
    Funkin::UI::UIRenderer& ui = Funkin::UI::UIRenderer::get();
    Funkin::Input::Input& input = Funkin::Input::Input::get();
    GAL::UI::DX12TextRenderer& txt = GAL::UI::DX12TextRenderer::get();
    Funkin::UI::ProjectUI projectUI;
    GAL::RenderPassDesc renderPass{};
    GAL::ColorAttachment colorAttachment{};
};

void setupCallbacks(Funkin::Core::Engine& engine, AppState& state, GAL::DX12Gal* dx12) {
    engine.setPreResizeCallback([&]() {
        LOG_REND("pre-resize: synchronizing GPU");
        dx12->waitIdle();
        state.txt.releaseBuffers();
        state.txt.flushD3D11();
    });

    engine.setResizeCallback([&](uint32_t w, uint32_t h) {
        if (w == 0 || h == 0) return;
        LOG_REND("resize: {}x{}", w, h);
        state.ui.resize(w, h);
        state.projectUI.onResize(w, h);
        state.txt.resize(dx12->swapchain(), 2, w, h);
    });

    engine.setFrameCallback([&]() {
        state.input.syncMousePosition();
        const auto& s = state.input.state();
        
        state.projectUI.update({ s.mouseX, s.mouseY }, s.mouseButtons[(size_t)Funkin::Input::MouseButton::Left]);

        auto* gal = engine.gal();
        auto size = gal->swapchainSize();

        if (size.x <= 0 || size.y <= 0) return;

        gal->beginRenderPass(state.renderPass);
        gal->setViewport({ 0, 0, size.x, size.y });
        gal->setScissor({ 0, 0, size.x, size.y });

        state.ui.beginFrame();
        state.projectUI.draw();
        state.ui.flush();
        gal->endRenderPass();
    });

    dx12->setPrePresentCallback([&](uint32_t frameIdx) {
        state.txt.beginDraw(frameIdx);
        state.ui.flushText();
        state.txt.endDraw(frameIdx);
    });
}

static int run() {
    auto& engine = Funkin::Core::Engine::get();
    AppState state;

    Funkin::Core::EngineConfig cfg {
        .title = "Funkin Project",
        .width = 900, 
        .height = 600,
        .vsync = true 
    };

    if (!engine.init(cfg)) {
        LOG_ERR("Engine initialization failed");
        return -1;
    }

    auto* dx12 = static_cast<GAL::DX12Gal*>(engine.gal());
    auto size = dx12->swapchainSize();

    Shader::ShaderLoader::get().init(Shader::ShaderBackend::DX12);
    state.ui.init(dx12, (uint32_t)size.x, (uint32_t)size.y);
    state.projectUI.init();
    state.projectUI.onResize((uint32_t)size.x, (uint32_t)size.y);

    state.txt.setFontFile(L"fonts/reg.ttf");
    state.txt.init(dx12->device(), dx12->queue(), dx12->swapchain(), 2, (uint32_t)size.x, (uint32_t)size.y);

    state.colorAttachment = {
        .loadOp     = GAL::LoadOp::Clear,
        .storeOp    = GAL::StoreOp::Store,
        .clearColor = { .07f, .07f, .07f, 1.0f }
    };

    state.renderPass = {
        .colorAttachments   = &state.colorAttachment,
        .colorCount         = 1,
        .useSwapchainTarget = true
    };

    state.input.bind("test", Funkin::Input::KeyCode::W);
    setupCallbacks(engine, state, dx12);

    while (engine.isRunning()) {
        if (!engine.processEvents()) break;

        if (state.input.justDown("test")) {
            uint64_t eventTime = state.input.getLastTimestamp("test");
            uint64_t now = state.input.getNow();
            LOG_INFO("latency: {:.4f} ms", (double)(now - eventTime) * 1e-6);
        }

        engine.tickFrame();
    }

    dx12->waitIdle();
    state.projectUI.shutdown();
    state.ui.shutdown();
    Shader::ShaderLoader::get().shutdown();
    engine.shutdown();

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    return run();
}
#else
int main() { return run(); }
#endif