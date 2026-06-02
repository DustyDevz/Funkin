// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

// holy fucking shit
// this file is a fucking mess, i'm sorry :(
// it WILL be fixed soon !!!
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <imgui.h>
#include "shared/log.hpp"
#include "input/input.hpp"
#include "settings.hpp"
#include "app/debug.hpp"
#include "projects/project.hpp"
#include "projects/launcher.hpp"
#include "registry.hpp"
#include "filesystem/filesystem.hpp"
#include "assets/assets.hpp"
#include "shaders/sprites/sprite_shader.hpp"
#include "shaders/shader_job_queue.hpp"
#include "shaders/shader_program.hpp"
#include "cache/cache.hpp"
#include "renderer/sprite/sprite_batch.hpp"
#include "renderer/sprite/sprite.hpp"

void DrawFileAssociationModal(bool& showModal) {
    if (showModal) {
        ImGui::OpenPopup("File Association");
    }

    if (ImGui::BeginPopupModal("File Association", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped(
            "Would you like for project files to be opened with the editor when double clicked? "
            "You can change this later in the settings."
        );
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            Funkin::App::ApplyFileAssociation();
            Funkin::Filesystem::writeString("local://data.json", "{\n  \"associated\": true\n}");

            showModal = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("No", ImVec2(120, 0))) {
            Funkin::Filesystem::writeString("local://data.json", "{\n  \"associated\": false\n}");

            showModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

int main(int argc, char** argv) {
    Funkin::Filesystem::init();
    Funkin::Cache::init();

    bool showAssociationPrompt = false;
    if (Funkin::Filesystem::readString("local://data.json").empty()) {
        showAssociationPrompt = true;
    }

    if (argc > 1) {
        std::filesystem::path target(argv[1]);
        if (target.filename() == "project.funkin") {
            if (Funkin::App::Project::get().load(target)) {
                LOG_PRINT("Project loaded from command line: {}", target.string());
            }
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        LOG_ERR("SDL init failed: {}", SDL_GetError());
        return 1;
    }

    Funkin::Settings appSettings;
    SDL_Window* window = SDL_CreateWindow("FNF CPP | initializing...", appSettings.windowWidth, appSettings.windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOG_ERR("SDL window failed: {}", SDL_GetError());
        return 1;
    }

    LOG_PRINT("SDL window OK");

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LOG_PRINT("HWND = {}", hwnd);

    bgfx::PlatformData pd{};
    pd.nwh = hwnd;
    pd.ndt = nullptr;
    bgfx::setPlatformData(pd);

    bgfx::renderFrame();

    bgfx::Init init{};
    init.platformData      = pd;
    init.resolution.width  = appSettings.windowWidth;
    init.resolution.height = appSettings.windowHeight;
    init.resolution.reset  = BGFX_RESET_VSYNC;
    init.type              = bgfx::RendererType::Vulkan;

    LOG_PRINT("bgfx initializing...");

    if (!bgfx::init(init)) {
        LOG_CRIT("bgfx init failed");
        return 1;
    }
    
    Funkin::DebugManager::init(window, 255);
    Funkin::Assets::AssetManager::get().init();
    Funkin::Shader::Sprites::init();

    const char* name = bgfx::getRendererName(bgfx::getRendererType());
    LOG_PRINT("bgfx using: {}", name);
    SDL_SetWindowTitle(window, (std::string("FNF [") + name + "]").c_str());

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, appSettings.windowWidth, appSettings.windowHeight);

    Funkin::Input::Input& input = Funkin::Input::Input::get();
    input.init();
    input.setWindow(window);
    input.bind("test", Funkin::Input::KeyCode::G);
    input.bind("debug", Funkin::Input::KeyCode::F1);

    bool running = true;
    SDL_Event e;
    // TEMP
    Funkin::Renderer::Sprite testSprite;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            
            input.handleSDLEvent(e);
            Funkin::DebugManager::handleEvent(e);
            Funkin::Assets::AssetManager::get().update();

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                uint32_t w = (uint32_t)e.window.data1;
                uint32_t h = (uint32_t)e.window.data2;
                bgfx::reset(w, h, (appSettings.vsync == Funkin::Settings::VSyncMode::On) ? BGFX_RESET_VSYNC : 0);
                bgfx::setViewRect(0, 0, 0, (uint16_t)w, (uint16_t)h);
            }
        }

        input.update();

        if (input.justDown("test")) {
            uint64_t eventTime = input.getLastTimestamp("test");
            uint64_t now = input.getNow();
            LOG_PRINT("input latency: {:.4f} ms", (double)(now - eventTime) * 1e-6);
        }

        if (input.justDown("debug")) {
            Funkin::DebugManager::toggleDebugStats();
        }

        Funkin::Shader::tickShaderJobs();
        Funkin::DebugManager::beginFrame();

        // TEMP
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        Funkin::Renderer::SpriteBatch::get().begin(0, (uint32_t)w, (uint32_t)h);
        testSprite.draw();
        Funkin::Renderer::SpriteBatch::get().end();

        if (showAssociationPrompt) {
            DrawFileAssociationModal(showAssociationPrompt);
        }

        if (!Funkin::App::Project::get().isLoaded()) {
            if (Funkin::App::RunLauncher()) {
                LOG_PRINT("Project loaded");
                SDL_SetWindowTitle(window, (std::string("FNF - ") + Funkin::App::Project::get().name).c_str());
                testSprite.loadTextureAsync("images/ui/test.png"); // this shit ass
                testSprite.x       = 640.0f;
                testSprite.y       = 360.0f;
                testSprite.width   = 400.0f;
                testSprite.height  = 400.0f;
                testSprite.scaleX  = 6.f;
                testSprite.scaleY  = 6.f;
                testSprite.originX = 0.5f;
                testSprite.originY = 0.5f;
            }
        }

        Funkin::DebugManager::endFrame();

        bgfx::touch(0);
        bgfx::frame();
    }

    Funkin::Assets::AssetManager::get().shutdown();
    Funkin::DebugManager::shutdown();
    Funkin::Shader::Sprites::shutdown();
    Funkin::Shader::shutdownShaderJobs();
    input.shutdown();
    bgfx::shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}