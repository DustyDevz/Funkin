#include "core/engine.hpp"
#include "core/log.hpp"
#include "renderer/shader/shader_loader.hpp"
#include "renderer/shader/shader_types.hpp"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <cstdio>
    #include "renderer/dx12/dx12_renderer.hpp"
    #include "renderer/dx12/dx12_pipeline.hpp"
    #include "scene/components/component_test.hpp"
#endif

static int run() {
    Funkin::Core::EngineConfig cfg {
        .title = "Funkin",
        .width = 1280,
        .height = 720,
        .vsync = false,
        .renderer = Funkin::Core::RendererBackend::DX12
    };

    auto& engine = Funkin::Core::Engine::get();

    LOG_INFO("init: engine ({}x{})", cfg.width, cfg.height);
    if (!engine.init(cfg)) {
        LOG_ERR("fail: engine init");
        return -1;
    }

#ifdef _WIN32
    auto& renderer    = Funkin::Renderer::DX12::DX12Renderer::get();
    auto& shaderLoader = Funkin::Renderer::Shader::ShaderLoader::get();

    LOG_REND("init: shaders");
    shaderLoader.init(Funkin::Renderer::Shader::ShaderBackend::DX12);

    auto* vs = shaderLoader.get("test", Funkin::Renderer::Shader::ShaderStage::Vertex);
    auto* ps = shaderLoader.get("test", Funkin::Renderer::Shader::ShaderStage::Pixel);

    if (!vs || !ps) {
        LOG_ERR("fail: loading shader 'test'");
        return -1;
    }

    LOG_REND("init: dx12 pipeline");
    Funkin::Renderer::DX12::DX12_Pipeline pipeline;
    pipeline.init(
        renderer.device(),
        DXGI_FORMAT_R8G8B8A8_UNORM,
        { vs->bytecode.data(), vs->bytecode.size() },
        { ps->bytecode.data(), ps->bytecode.size() }
    );

    LOG_REND("init: test component");
    Funkin::Scene::Components::Test test;
    test.init(renderer.device());

    LOG_INFO("engine: running");
    while (engine.isRunning()) {
        engine.beginFrame();
        test.draw(renderer.cmdList(), pipeline, renderer.viewport(), renderer.scissor());
        engine.endFrame();
    }

    test.shutdown();
#else
    engine.run();
#endif

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
