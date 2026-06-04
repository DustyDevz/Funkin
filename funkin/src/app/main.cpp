// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <dwmapi.h>

#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QAbstractNativeEventFilter>
#include <QResizeEvent>
#include <QDockWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QPaintEngine>

#include "shared/log.hpp"
#include "input/input.hpp"
#include "settings.hpp"
#include "app/debug.hpp"
#include "project/project.hpp"
#include "ui/project/ui_project.hpp"
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

class ViewportWidget : public QWidget {
public:
    explicit ViewportWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAutoFillBackground(false);
        setFocusPolicy(Qt::StrongFocus);
    }

    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void paintEvent(QPaintEvent*) override {}

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override {
        if (eventType == "windows_generic_MSG") {
            MSG* msg = static_cast<MSG*>(message);
            if (msg->message == WM_ERASEBKGND) {
                *result = 1;
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
};

class RawInputFilter : public QAbstractNativeEventFilter {
public:
    std::function<void()> onRenderFrame;
    bool windowMoving = false;
    QWidget* viewportWidget = nullptr;

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override {
        if (eventType == "windows_generic_MSG") {
            MSG* msg = static_cast<MSG*>(message);

            if (msg->message == WM_ENTERSIZEMOVE) {
                windowMoving = true;
            }

            if (msg->message == WM_EXITSIZEMOVE) {
                windowMoving = false;
            }

            if (viewportWidget && viewportWidget->internalWinId()) {
                HWND vpHwnd = (HWND)viewportWidget->winId();
                if (msg->hwnd == vpHwnd) {
                    if (msg->message == WM_ERASEBKGND) {
                        *result = 1;
                        return true;
                    }

                    if (msg->message == WM_SIZE && bgfx::getRendererType() != bgfx::RendererType::Noop) {
                        int w = LOWORD(msg->lParam);
                        int h = HIWORD(msg->lParam);

                        if (w > 0 && h > 0) {
                            bgfx::reset((uint32_t)w, (uint32_t)h, BGFX_RESET_VSYNC);
                            bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 1.0f, 0);
                            bgfx::setViewRect(0, 0, 0, (uint16_t)w, (uint16_t)h);
                            if (onRenderFrame) {
                                onRenderFrame();
                            }
                        }
                    }

                    if (msg->message == WM_PAINT && windowMoving && bgfx::getRendererType() != bgfx::RendererType::Noop) {
                        if (onRenderFrame) {
                            onRenderFrame();
                        }
                    }
                }
            }

            if (msg->message == WM_INPUT) {
                Funkin::Platform::Input::handleRawInput(
                    (HRAWINPUT)msg->lParam,
                    Funkin::Input::Input::get().ring(),
                    Funkin::Input::Input::get().startTime(),
                    Funkin::Platform::Input::nanoTime()
                );
                return false;
            }

            extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
            ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam);
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

    if (!Funkin::App::Project::get().isLoaded()) {
        if (!Funkin::UI::Project::RunLauncher(nullptr)) {
            return 0;
        }
    }

    Funkin::Settings appSettings;
    LOG_PRINT("Qt platform: {}", QGuiApplication::platformName().toStdString());

    QMainWindow* editorWindow = new QMainWindow();
    editorWindow->setWindowTitle("Friday Night Funkin Engine");
    editorWindow->resize(appSettings.windowWidth, appSettings.windowHeight);
    
    editorWindow->setStyleSheet(
        "QMainWindow { background-color: #1a1a1a; }"
        "QDockWidget { background-color: #1a1a1a; color: #ffffff; }"
    );

    ViewportWidget* viewport = new ViewportWidget(editorWindow);
    editorWindow->setCentralWidget(viewport);
    rawInputFilter.viewportWidget = viewport;

    editorWindow->hide();

    HWND hwnd = (HWND)viewport->winId();
    HWND qtHwnd = (HWND)editorWindow->winId();
    
    LONG_PTR vpStyle = GetClassLongPtr(hwnd, GCL_STYLE);
    vpStyle &= ~(CS_HREDRAW | CS_VREDRAW);
    SetClassLongPtr(hwnd, GCL_STYLE, vpStyle);

    DWM_BLURBEHIND bb{};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = FALSE;
    DwmEnableBlurBehindWindow(qtHwnd, &bb);

    MARGINS margins = { 0, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(qtHwnd, &margins);
    Funkin::Platform::Input::registerRawInput(qtHwnd);

    bgfx::PlatformData pd{};
    pd.nwh = hwnd;
    pd.ndt = nullptr;
    bgfx::setPlatformData(pd);

    bgfx::renderFrame();

    bgfx::Init init{};
    init.platformData      = pd;
    init.resolution.width  = (uint32_t)viewport->width();
    init.resolution.height = (uint32_t)viewport->height();
    init.resolution.reset  = BGFX_RESET_VSYNC;
    init.type              = bgfx::RendererType::Vulkan;

    LOG_PRINT("bgfx initializing...");

    if (!bgfx::init(init)) {
        LOG_CRIT("bgfx init failed");
        return 1;
    }

    Funkin::DebugManager::init(hwnd, 255);
    Funkin::Assets::AssetManager::get().init();
    Funkin::Shader::Sprites::init();

    const char* name = bgfx::getRendererName(bgfx::getRendererType());
    LOG_PRINT("bgfx using: {}", name);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, (uint16_t)viewport->width(), (uint16_t)viewport->height());

    bgfx::touch(0);
    bgfx::frame();
    bgfx::frame();

    Funkin::Input::Input& input = Funkin::Input::Input::get();
    input.init();
    input.bind("test", Funkin::Input::KeyCode::G);
    input.bind("debug", Funkin::Input::KeyCode::F1);
    input.bind("up", Funkin::Input::KeyCode::W);
    input.bind("down", Funkin::Input::KeyCode::S);
    input.bind("left", Funkin::Input::KeyCode::A);
    input.bind("right", Funkin::Input::KeyCode::D);

    auto lastTime = std::chrono::high_resolution_clock::now();
    Funkin::Renderer::AnimatedSprite testSprite;

    LOG_PRINT("Project loaded: {}", Funkin::App::Project::get().name);
    editorWindow->setWindowTitle((std::string("FNF - ") + Funkin::App::Project::get().name).c_str());
    testSprite.loadAtlas("images/ui/test.xml", "test");
    testSprite.addAnimation("idle",  "idle",  24.f, true);
    testSprite.addAnimation("left",  "left",  24.f, false);
    testSprite.addAnimation("down",  "down",  24.f, false);
    testSprite.addAnimation("up",    "up",    24.f, false);
    testSprite.addAnimation("right", "right", 24.f, false);
    testSprite.play("idle");
    testSprite.onAnimComplete = [&](const std::string& anim) {
        testSprite.play("idle");
    };

    rawInputFilter.onRenderFrame = [&]() {
        if (!running || !editorWindow->isVisible()) {
            return;
        }

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        Funkin::Assets::AssetManager::get().update();
        input.update();
        testSprite.update(dt);

        if (input.justDown("test")) {
            uint64_t eventTime = input.getLastTimestamp("test");
            uint64_t n = input.getNow();
            LOG_PRINT("input latency: {:.4f} ms", (double)(n - eventTime) * 1e-6);
        }

        if (input.justDown("debug"))
            Funkin::DebugManager::toggleDebugStats();

        Funkin::Shader::tickShaderJobs();
        Funkin::DebugManager::beginFrame();

        uint32_t w = (uint32_t)viewport->width();
        uint32_t h = (uint32_t)viewport->height();
        if (w > 0 && h > 0) {
            Funkin::Renderer::SpriteBatch::get().begin(0, w, h);
            testSprite.draw();
            Funkin::Renderer::SpriteBatch::get().end();
        }

        if (showAssociationPrompt)
            DrawFileAssociationModal(showAssociationPrompt);

        if (input.justDown("up"))         testSprite.play("up", true);
        else if (input.justDown("down"))  testSprite.play("down", true);
        else if (input.justDown("left"))  testSprite.play("left", true);
        else if (input.justDown("right")) testSprite.play("right", true);

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

    QTimer::singleShot(1500, [&]() {
        if (running) {
            editorWindow->show();
            qtApp.processEvents(QEventLoop::AllEvents);
            renderTimer.start();
        }
    });

    int result = qtApp.exec();
    Funkin::Assets::AssetManager::get().shutdown();
    Funkin::DebugManager::shutdown();
    Funkin::Shader::Sprites::shutdown();
    Funkin::Shader::shutdownShaderJobs();
    input.shutdown();
    bgfx::shutdown();
    
    delete editorWindow;
    return result;
}