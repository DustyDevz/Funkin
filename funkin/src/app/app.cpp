// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "app.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <dwmapi.h>
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <QTimer>
#include <QCursor>
#include <QProgressBar>

#include "shared/log.hpp"
#include "app/debug.hpp"
#include "app/project/project.hpp"
#include "app/registry.hpp"
#include "filesystem/filesystem.hpp"
#include "assets/assets.hpp"
#include "cache/cache.hpp"
#include "input/input.hpp"
#include "shaders/sprites/sprite_shader.hpp"
#include "shaders/shader_job_queue.hpp"
#include "renderer/sprite/sprite_batch.hpp"
#include "renderer/sprite/animated_sprite.hpp"
#include "renderer/sprite/sprite.hpp"
#include "renderer/camera/camera.hpp"
#include "platform/win32/win32_input.hpp"
#include "platform/win32/win32_window.hpp"
#include "ui/project/ui_project.hpp"
#include "ui/editor/ui_editor.hpp"
#include "ui/ui_log.hpp"
#include "settings.hpp"
#include "cache/project_cache.hpp"
#include "renderer/audio/audio_manager.hpp"
#include "renderer/audio/audio_source.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

namespace Funkin::App {

static void drawFileAssociationModal(bool& showModal) {
    if (showModal) ImGui::OpenPopup("File Association");

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

static bool initProject(int argc, char** argv) {
    Funkin::Filesystem::init();
    Funkin::Cache::init();

    if (argc > 1) {
        std::filesystem::path target(argv[1]);
        if (target.filename() == "project.funkin") {
            if (Funkin::App::Project::get().load(target))
                LOG_PRINT("Project loaded from command line: {}", target.string());
        }
    }

    if (!Funkin::App::Project::get().isLoaded()) {
        if (!Funkin::UI::Project::RunLauncher(nullptr))
            return false;
    }

    return true;
}

static bool initBgfx(HWND hwnd, uint32_t w, uint32_t h) {
    bgfx::PlatformData pd{};
    pd.nwh = hwnd;
    pd.ndt = nullptr;
    bgfx::setPlatformData(pd);
    bgfx::renderFrame();

    bgfx::Init init{};
    init.callback          = Funkin::DebugManager::getCallBack();
    init.platformData      = pd;
    init.resolution.width  = w;
    init.resolution.height = h;
    init.resolution.reset  = BGFX_RESET_VSYNC;
    init.type              = bgfx::RendererType::Vulkan;

    LOG_PRINT("bgfx initializing...");
    return bgfx::init(init);
}

static void setupWindow(HWND hwnd, HWND qtHwnd) {
    SetClassLongPtr(qtHwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(NULL_BRUSH));

    LONG_PTR vpStyle = GetClassLongPtr(hwnd, GCL_STYLE);
    vpStyle &= ~(CS_HREDRAW | CS_VREDRAW);
    SetClassLongPtr(hwnd, GCL_STYLE, vpStyle);

    DWM_BLURBEHIND bb{};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = FALSE;
    DwmEnableBlurBehindWindow(qtHwnd, &bb);

    MARGINS margins = { 0, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(qtHwnd, &margins);
}

int run(int argc, char** argv, QApplication& qtApp) {
    bool running = true;

    RawInputFilter rawInputFilter;
    qtApp.installNativeEventFilter(&rawInputFilter);

    QObject::connect(&qtApp, &QGuiApplication::lastWindowClosed, [&]() {
        running = false;
    });

    if (!initProject(argc, argv))
        return 0;

    if (Funkin::App::Project::get().isFirstRun()) {
        auto& cache = Funkin::Cache::ProjectCache::get();

        QDialog loadingDialog;
        loadingDialog.setWindowTitle("Preparing Project");
        loadingDialog.setFixedSize(380, 90);
        loadingDialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

        auto* layout   = new QVBoxLayout(&loadingDialog);
        auto* label    = new QLabel("Building texture cache...", &loadingDialog);
        auto* bar      = new QProgressBar(&loadingDialog);
        auto* fileLabel = new QLabel("", &loadingDialog);

        label->setAlignment(Qt::AlignCenter);
        fileLabel->setAlignment(Qt::AlignCenter);
        fileLabel->setStyleSheet("color: #666666; font-size: 10px;");
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setTextVisible(false);

        layout->addWidget(label);
        layout->addWidget(bar);
        layout->addWidget(fileLabel);
        
        QTimer pollTimer;
        pollTimer.setInterval(50);
        QObject::connect(&pollTimer, &QTimer::timeout, [&]() {
            auto& p = cache.progress();
            bar->setValue((int)p.percent());

            {
                std::lock_guard lk(p.mutex);
                if (!p.currentFile.empty())
                    fileLabel->setText(QString::fromStdString(p.currentFile));
            }

            if (p.done.load()) {
                pollTimer.stop();
                loadingDialog.accept();
            }
        });

        cache.warmAsync(
            Funkin::App::Project::get().getAssets(),
            Funkin::App::Project::get().getCacheDir()
        );

        pollTimer.start();
        loadingDialog.exec();
    }

    bool showAssociationPrompt = Funkin::Filesystem::readString("local://data.json").empty();

    Funkin::Settings appSettings;
    LOG_PRINT("Qt platform: {}", QGuiApplication::platformName().toStdString());

    auto* editorWindow = new Funkin::UI::Editor::EditorWindow();
    auto* viewport     = new ViewportWidget(editorWindow);
    rawInputFilter.viewportWidget = viewport;

    editorWindow->setGameViewport(viewport);
    editorWindow->hide();
    editorWindow->setWindowState(Qt::WindowMaximized);

    QObject::connect(editorWindow, &Funkin::UI::Editor::EditorWindow::activeTabChanged,
    [&](const QString& tabName) {
        bool isGame = (tabName == "Game");
        viewport->setVisible(isGame);
        editorWindow->blankWidget()->setVisible(!isGame);
    });

    HWND hwnd   = (HWND)viewport->winId();
    HWND qtHwnd = (HWND)editorWindow->winId();

    setupWindow(hwnd, qtHwnd);
    Funkin::Platform::Input::registerRawInput(qtHwnd);

    if (!initBgfx(hwnd, (uint32_t)viewport->width(), (uint32_t)viewport->height())) {
        LOG_CRIT("bgfx init failed");
        return 1;
    }

    editorWindow->setRendererLabel(bgfx::getRendererName(bgfx::getRendererType()));
    LOG_PRINT("bgfx using: {}", bgfx::getRendererName(bgfx::getRendererType()));

    Funkin::DebugManager::init(hwnd, 255);
    Funkin::Assets::AssetManager::get().init();
    Funkin::Shader::Sprites::init();
    Funkin::Audio::AudioManager::get().init();

    auto& camera = Funkin::Renderer::Camera::CameraManager::get();
    camera.addLayer("editor", 0, &camera.editor());

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, (uint16_t)viewport->width(), (uint16_t)viewport->height());

    auto& input = Funkin::Input::Input::get();
    input.init();
    input.bind("debug", Funkin::Input::KeyCode::F1);
    input.bind("up",    Funkin::Input::KeyCode::W);
    input.bind("down",  Funkin::Input::KeyCode::S);
    input.bind("left",  Funkin::Input::KeyCode::A);
    input.bind("right", Funkin::Input::KeyCode::D);
    input.bind("char_up",    Funkin::Input::KeyCode::I);
    input.bind("char_down",  Funkin::Input::KeyCode::K);
    input.bind("char_left",  Funkin::Input::KeyCode::J);
    input.bind("char_right", Funkin::Input::KeyCode::L);
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    Funkin::Renderer::AnimatedSprite testSprite;
    Funkin::Renderer::Sprite         bgTest;

    LOG_PRINT("Project loaded: {}", Funkin::App::Project::get().getName());
    editorWindow->setWindowTitle(
        QString("FNF - %1").arg(Funkin::App::Project::get().getName().c_str()));

    // CAMERA LAYER TEST
    auto* testLayer = camera.addLayer("test", 1, &camera.editor());
    // testLayer->scrollX = 1.55;
    // testLayer->scrollY = 1.55;

    // SPRITE TEST
    bgTest.loadTexture("images/ui/thing.png");
    bgTest.setScale(5.f);
    bgTest.x -= 300;
    //bgTest.flipY = true;
    bgTest.viewId = testLayer->viewId;

    testSprite.loadAtlas("images/ui/bf.xml", "test");
    testSprite.addAnimation("idle",  "BF IDLE instance",       24.f, true);
    testSprite.addAnimation("left",  "BF LEFT NOTE instance",  24.f, false);
    testSprite.addAnimation("down",  "BF DOWN NOTE instance",  24.f, false);
    testSprite.addAnimation("up",    "BF UP NOTE instance",    24.f, false);
    testSprite.addAnimation("right", "BF RIGHT NOTE instance", 24.f, false);
    testSprite.play("idle");
    testSprite.setScale(6.f);
    testSprite.pixel = true;
    testSprite.onAnimComplete = [&](const std::string&) {
        if (!input.isDown("up") && !input.isDown("down") &&
            !input.isDown("left") && !input.isDown("right") &&
            !input.isDown("char_up") && !input.isDown("char_down") &&
            !input.isDown("char_left") && !input.isDown("char_right"))
            testSprite.play("idle");
    };

    // AUDIO TEST
    Funkin::Audio::AudioSource audioTest;
    audioTest.setLoop(true);
    audioTest.load("audio/test.ogg");
    audioTest.setVolume(.1);
    audioTest.play();

    // TEST DEBUGGING
    // QTimer* testTimer = new QTimer(&qtApp);
    // testTimer->setInterval(1000);
    // QObject::connect(testTimer, &QTimer::timeout, []() {
    //     // is this smart? fuck no wtf
    //     static const char* words[]    = { "asdiosadn", "fkljsdhf", "xzqwerty", "ploiuyt", "mnbvcxz" };
    //     static const char* prefixes[] = { "Loading", "Failed to load", "Initializing", "Destroyed", "Created" };
    //     std::string msg = std::string(prefixes[rand() % 5]) + " " + words[rand() % 5];
    //     switch (rand() % 3) {
    //         case 0: UI_ENGINE_INFO(msg);  break;
    //         case 1: UI_ENGINE_WARN(msg);  break;
    //         case 2: UI_ENGINE_ERROR(msg); break;
    //     }
    // });
    // testTimer->start();

    rawInputFilter.onRenderFrame = [&]() {
        if (!running || !editorWindow->isVisible()) return;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        uint32_t w = (uint32_t)viewport->width();
        uint32_t h = (uint32_t)viewport->height();

        Funkin::Assets::AssetManager::get().update();
        input.update();
        testSprite.update(dt);

        // camera
        if (input.isDown("up"))    camera.editor().onKeyPan(0.f, -1.f, dt);
        if (input.isDown("down"))  camera.editor().onKeyPan(0.f,  1.f, dt);
        if (input.isDown("left"))  camera.editor().onKeyPan(-1.f, 0.f, dt);
        if (input.isDown("right")) camera.editor().onKeyPan( 1.f, 0.f, dt);

        if (input.state().scrollY != 0.0f) {
            QPoint localMouse = viewport->mapFromGlobal(QCursor::pos());
            camera.editor().onScroll(
                input.state().scrollY,
                (float)localMouse.x(), (float)localMouse.y(),
                (float)w, (float)h
            );
        }

        camera.update(dt);

        if (input.justDown("char_up"))    testSprite.play("up", true);
        if (input.justDown("char_down"))  testSprite.play("down", true);
        if (input.justDown("char_left"))  testSprite.play("left", true);
        if (input.justDown("char_right")) testSprite.play("right", true);

        if (input.justDown("debug"))
            Funkin::DebugManager::toggleDebugStats();

        Funkin::Shader::tickShaderJobs();
        Funkin::DebugManager::beginFrame();

        if (w > 0 && h > 0) {
            camera.applyAll(w, h);
            Funkin::Renderer::SpriteBatch::get().begin(0, w, h);
            bgTest.draw();
            testSprite.draw();
            Funkin::Renderer::SpriteBatch::get().end();
        }

        if (showAssociationPrompt)
            drawFileAssociationModal(showAssociationPrompt);

        const bgfx::Stats* stats = bgfx::getStats();
        double cpuFreq = (double)stats->cpuTimerFreq;
        float fps  = cpuFreq > 0 ? (float)(cpuFreq / (stats->cpuTimeEnd - stats->cpuTimeBegin)) : 0.f;
        float vram = stats->gpuMemoryUsed / (1024.f * 1024.f);

        PROCESS_MEMORY_COUNTERS_EX pmc{};
        pmc.cb = sizeof(pmc);
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        float memMB = pmc.WorkingSetSize / (1024.f * 1024.f);

        editorWindow->updateStats(fps, vram, memMB);

        Funkin::DebugManager::endFrame();
        bgfx::touch(0);
        bgfx::frame();
    };

    QTimer renderTimer;
    renderTimer.setInterval(0);
    QObject::connect(&renderTimer, &QTimer::timeout, [&]() {
        if (!running || !editorWindow->isVisible()) {
            renderTimer.stop();
            qtApp.quit();
            return;
        }
        rawInputFilter.onRenderFrame();
    });

    // waits for the window and renderer to finish so its not pure white
    // ill create a check another time instead of a timer
    QTimer::singleShot(1250, [&]() {
        if (running) {
            editorWindow->show();
            editorWindow->showMaximized();
            qtApp.processEvents(QEventLoop::AllEvents);
            renderTimer.start();
        }
    });

    int result = qtApp.exec();
    Funkin::Assets::AssetManager::get().shutdown();
    Funkin::Renderer::SpriteBatch::get().shutdown();
    Funkin::Shader::Sprites::shutdown();
    Funkin::Shader::shutdownShaderJobs();
    Funkin::DebugManager::shutdown();
    bgfx::shutdown();
    delete editorWindow;
    Funkin::Audio::AudioManager::get().shutdown();
    input.shutdown();

    return result;
}

}