// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "debug.hpp"
#include <string>
#include "shared/log.hpp"
#include "imgui/imgui_impl_bgfx.hpp"
#include "imgui/imgui_impl_sdl3.hpp"

struct BgfxCallback : public bgfx::CallbackI {
    void fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str) override {
        LOG_CRIT("{}({}): {}", filePath, line, str);
    }

    void traceVargs(const char* filePath, uint16_t line, const char* format, va_list argList) override {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), format, argList);
        std::string_view sv = buf;
        while (!sv.empty() && (sv.back() == '\n' || sv.back() == '\r'))
            sv.remove_suffix(1);
        if (!sv.empty())
            LOG_PRINT("{}", sv);
    }
    
    void profilerBegin(const char*, uint32_t, const char*, uint16_t) override {}
    void profilerBeginLiteral(const char*, uint32_t, const char*, uint16_t) override {}
    void profilerEnd() override {}
    uint32_t cacheReadSize(uint64_t) override { return 0; }
    bool cacheRead(uint64_t, void*, uint32_t) override { return false; }
    void cacheWrite(uint64_t, const void*, uint32_t) override {}
    void screenShot(const char*, uint32_t, uint32_t, uint32_t, bgfx::TextureFormat::Enum, const void*, uint32_t, bool) override {}
    void captureBegin(uint32_t, uint32_t, uint32_t, bgfx::TextureFormat::Enum, bool) override {}
    void captureEnd() override {}
    void captureFrame(const void*, uint32_t) override {}
};

namespace Funkin::DebugManager {
    static BgfxCallback s_callback;
    static bool s_enabled = false;
    static bgfx::ViewId s_viewId = 255;

    void init(SDL_Window* window, bgfx::ViewId viewId) {
        s_viewId = viewId;

        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        ImGui_SDL3::init(window);
        ImGui_BGFX::init(s_viewId);
    }

    void handleEvent(const SDL_Event& e) {
        ImGui_SDL3::processEvent(e);
    }

    void beginFrame() {
        ImGui_SDL3::newFrame();
        ImGui_BGFX::newFrame();
        ImGui::NewFrame();

        // ? only use for debugging imgui issues
        //ImGui::ShowDemoWindow();
    }

    void endFrame() {
        ImGui::Render();
        ImGui_BGFX::render(ImGui::GetDrawData());
    }

    void shutdown() {
        ImGui_BGFX::shutdown();
        ImGui_SDL3::shutdown();
        ImGui::DestroyContext();
    }

    void toggleDebugStats() {
        s_enabled = !s_enabled;
        bgfx::setDebug(s_enabled ? BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS : 0);
    }

    bool isEnabled() {
        return s_enabled;
    }
}