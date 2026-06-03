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

#include <QApplication>
#include <QWindow>
#include <QWidget>
#include <QMainWindow>
#include <QAbstractNativeEventFilter>
#include <QResizeEvent>

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
#include "renderer/sprite/animated_sprite.hpp"
#include "platform/win32/win32_input.hpp"

class ViewportResizeFilter : public QObject {
public:
    std::function<void(int,int)> onResize;
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::Resize) {
            auto* re = static_cast<QResizeEvent*>(event);
            if (onResize) onResize(re->size().width(), re->size().height());
        }
        return QObject::eventFilter(obj, event);
    }
};

class RawInputFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override {
        if (eventType == "windows_generic_MSG") {
            MSG* msg = static_cast<MSG*>(message);
            if (msg->message == WM_INPUT) {
                Funkin::Platform::Input::handleRawInput(
                    (HRAWINPUT)msg->lParam,
                    Funkin::Input::Input::get().ring(),
                    Funkin::Input::Input::get().startTime(),
                    Funkin::Platform::Input::nanoTime()
                );
                return false;
            }
        }
        return false;
    }
};

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
    bool running = true;

    QApplication qtApp(argc, argv);
    RawInputFilter rawInputFilter;
    qtApp.installNativeEventFilter(&rawInputFilter);

    QObject::connect(&qtApp, &QGuiApplication::lastWindowClosed, [&]() {
        running = false;
    });

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
    SDL_Window* window = SDL_CreateWindow("FNF CPP | initializing...", appSettings.windowWidth, appSettings.windowHeight, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN
    );

    if (!window) {
        LOG_ERR("SDL window failed: {}", SDL_GetError());
        return 1;
    }

    LOG_PRINT("SDL window OK");

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    LOG_PRINT("HWND = {}", hwnd);
    LOG_PRINT("Qt platform: {}", QGuiApplication::platformName().toStdString());

    QMainWindow* editorWindow = new QMainWindow();
    editorWindow->setWindowTitle("FNF CPP | initializing...");
    editorWindow->resize(appSettings.windowWidth, appSettings.windowHeight);

    QWindow* sdlQWindow = QWindow::fromWinId((WId)(quintptr)hwnd);
    editorWindow->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QWidget* viewport = QWidget::createWindowContainer(sdlQWindow, editorWindow);    
    editorWindow->setCentralWidget(viewport);
    viewport->setFocusPolicy(Qt::StrongFocus);
    viewport->setAttribute(Qt::WA_NativeWindow);
    viewport->setFocus();
    //editorWindow->installEventFilter

    ViewportResizeFilter* resizeFilter = new ViewportResizeFilter();
    resizeFilter->onResize = [&](int w, int h) {
        bgfx::reset((uint32_t)w, (uint32_t)h, BGFX_RESET_VSYNC);
        bgfx::setViewRect(0, 0, 0, (uint16_t)w, (uint16_t)h);
    };
    viewport->installEventFilter(resizeFilter);

    HWND qtHwnd = (HWND)editorWindow->winId();
    Funkin::Platform::Input::registerRawInput(qtHwnd);

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

    editorWindow->show();
    SDL_ShowWindow(window);
    
    Funkin::DebugManager::init(window, 255);
    Funkin::Assets::AssetManager::get().init();
    Funkin::Shader::Sprites::init();

    const char* name = bgfx::getRendererName(bgfx::getRendererType());
    LOG_PRINT("bgfx using: {}", name);
    editorWindow->setWindowTitle((std::string("FNF [") + name + "]").c_str());

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, appSettings.windowWidth, appSettings.windowHeight);

    Funkin::Input::Input& input = Funkin::Input::Input::get();
    input.init();
    input.setWindow(window);
    input.bind("test", Funkin::Input::KeyCode::G);
    input.bind("debug", Funkin::Input::KeyCode::F1);
    input.bind("up", Funkin::Input::KeyCode::W);
    input.bind("down", Funkin::Input::KeyCode::S);
    input.bind("left", Funkin::Input::KeyCode::A);
    input.bind("right", Funkin::Input::KeyCode::D);

    SDL_Event e;
    auto lastTime = std::chrono::high_resolution_clock::now();
    // TEMP
    Funkin::Renderer::AnimatedSprite testSprite;

    while (running && editorWindow->isVisible()) {
        // delta time
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        qtApp.processEvents();

        while (SDL_PollEvent(&e)) {
            input.handleSDLEvent(e);
            Funkin::DebugManager::handleEvent(e);
        }

        Funkin::Assets::AssetManager::get().update();
        input.update();
        testSprite.update(dt);

        if (input.justDown("test")) {
            uint64_t eventTime = input.getLastTimestamp("test");
            uint64_t now = input.getNow();
            LOG_PRINT("input latency: {:.4f} ms", (double)(now - eventTime) * 1e-6);
        }

        if (input.justDown("debug"))
            Funkin::DebugManager::toggleDebugStats();

        Funkin::Shader::tickShaderJobs();
        Funkin::DebugManager::beginFrame();

        uint32_t w = (uint32_t)viewport->width();
        uint32_t h = (uint32_t)viewport->height();
        Funkin::Renderer::SpriteBatch::get().begin(0, w, h);
        testSprite.draw();
        Funkin::Renderer::SpriteBatch::get().end();

        if (showAssociationPrompt)
            DrawFileAssociationModal(showAssociationPrompt);

        if (!Funkin::App::Project::get().isLoaded()) {
            if (Funkin::App::RunLauncher()) {
                LOG_PRINT("Project loaded");
                editorWindow->setWindowTitle((std::string("FNF - ") + Funkin::App::Project::get().name).c_str());
                testSprite.loadAtlas("images/ui/test.xml", "test");
                testSprite.addAnimation("idle",  "idle",  24.f, true);
                testSprite.addAnimation("left",  "left",  24.f, false);
                testSprite.addAnimation("down",  "down",  24.f, false);
                testSprite.addAnimation("up",    "up",    24.f, false);
                testSprite.addAnimation("right", "right", 24.f, false);
                testSprite.play("idle");

                testSprite.onAnimComplete = [&](const std::string& name) {
                    testSprite.play("idle");
                };
            }
        }

        if (input.justDown("up"))
            testSprite.play("up", true);
        else if (input.justDown("down"))
            testSprite.play("down", true);
        else if (input.justDown("left"))
            testSprite.play("left", true);
        else if (input.justDown("right"))
            testSprite.play("right", true);

        Funkin::DebugManager::endFrame();

        bgfx::touch(0);
        bgfx::frame();
    }

    // stops window flash on exit
    SDL_HideWindow(window);

    Funkin::Assets::AssetManager::get().shutdown();
    Funkin::DebugManager::shutdown();
    Funkin::Shader::Sprites::shutdown();
    Funkin::Shader::shutdownShaderJobs();
    input.shutdown();
    bgfx::shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    delete editorWindow;
    return 0;
}