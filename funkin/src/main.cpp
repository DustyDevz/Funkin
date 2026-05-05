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

namespace GAL    = Funkin::Renderer::GAL;
namespace Shader = Funkin::Renderer::Shader;

static int run() {
    Funkin::Core::EngineConfig cfg {
        .title  = "Funkin Project",
        .width  = 900,
        .height = 600,
        .vsync  = true,
    };

    auto& engine = Funkin::Core::Engine::get();

    LOG_INFO("init: engine ({}x{})", cfg.width, cfg.height);
    if (!engine.init(cfg)) {
        LOG_ERR("fail: engine init");
        return -1;
    }

    auto* gal = engine.gal();

    LOG_INFO("init: shaders");
    auto& shaderLoader = Shader::ShaderLoader::get();
    shaderLoader.init(Shader::ShaderBackend::DX12);

    LOG_INFO("init: ui renderer");
    auto& ui = Funkin::UI::UIRenderer::get();
    ui.init(gal, cfg.width, cfg.height);

    LOG_INFO("input: test binded");
    auto& input = Funkin::Input::Input::get();
    input.bind("test", Funkin::Input::KeyCode::W);

    LOG_INFO("init: project manager");
    Funkin::UI::ProjectUI ProjectUI;
    ProjectUI.init();

    {
        auto size = gal->swapchainSize();
        ui.resize((uint32_t)size.x, (uint32_t)size.y);
        ProjectUI.onResize((uint32_t)size.x, (uint32_t)size.y);
    }

    engine.setResizeCallback([&](uint32_t w, uint32_t h) {
        ui.resize(w, h);
        ProjectUI.onResize(w, h);
    });

    engine.setFrameCallback([&]() {
        auto& input   = Funkin::Input::Input::get();
        input.syncMousePosition();

        Funkin::Vec2 mouse = { input.state().mouseX, input.state().mouseY };
        bool clicked = input.state().mouseButtons[(size_t)Funkin::Input::MouseButton::Left];

        ProjectUI.update(mouse, clicked);

        GAL::ColorAttachment color{};
        color.clearColor = { .07f, .07f, .07f, 1.0f };
        color.loadOp     = GAL::LoadOp::Clear;
        color.storeOp    = GAL::StoreOp::Store;

        GAL::RenderPassDesc rp{};
        rp.useSwapchainTarget  = true;
        rp.colorAttachments    = &color;
        rp.colorCount          = 1;

        gal->beginRenderPass(rp);
        
        auto size = gal->swapchainSize();
        gal->setViewport({ 0.0f, 0.0f, size.x, size.y });
        gal->setScissor ({ 0.0f, 0.0f, size.x, size.y });

        ui.beginFrame();
        ProjectUI.draw();
        ui.flush();

        gal->endRenderPass();
    });

    LOG_INFO("engine: running");
    while (engine.isRunning()) {
        if (!engine.processEvents()) break;
        engine.tickFrame();

        if (input.justDown("test")) {
            uint64_t eventTime = input.getLastTimestamp("test");
            uint64_t now = input.getNow();
            double latencyMs = (now >= eventTime) ? (double)(now - eventTime) / 1'000'000.0 : 0.0;
            LOG_INFO("Now: {} | Event: {} | Latency: {:.4f} ms", now, eventTime, latencyMs);
        }
    }

    gal->waitIdle();

    ProjectUI.shutdown();
    ui.shutdown();
    shaderLoader.shutdown();
    engine.shutdown();

    LOG_INFO("engine: shutdown");
    return 0;
}

#ifdef _WIN32
    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        return run();
    }
#else
    int main() {
        return run();
    }
#endif