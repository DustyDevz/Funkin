#include "core/engine.hpp"
#include "core/log.hpp"
#include "renderer/shader/shader_loader.hpp"
#include "renderer/shader/shader_types.hpp"
#include "scene/components/component_test.hpp"
#include "ui/ui_renderer.hpp"
#include <input/input.hpp>

namespace GAL    = Funkin::Renderer::GAL;
namespace Shader = Funkin::Renderer::Shader;

static int run() {
    Funkin::Core::EngineConfig cfg {
        .title  = "Funkin",
        .width  = 1280,
        .height = 720,
        .vsync  = false,
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

    engine.setFrameCallback([&]() {
        GAL::ColorAttachment color{};
        color.clearColor = { 0.1f, 0.2f, 0.3f, 1.0f };
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

        gal->endRenderPass();
    });

    LOG_INFO("input: test binded");
    auto& input = Funkin::Input::Input::get();
    input.bind("test", Funkin::Input::KeyCode::W);

    LOG_INFO("engine: running");
    while (engine.isRunning()) {
        if (!engine.processEvents()) break;
        engine.tickFrame();

        if (input.isDown("test")) {
            LOG_INFO("input: test hit!");
        }
    }

    gal->waitIdle();

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
