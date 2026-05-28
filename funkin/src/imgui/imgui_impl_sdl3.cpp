// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "imgui_impl_sdl3.hpp"

struct SDL3Data {
    SDL_Window* window   = nullptr;
    SDL_Cursor* cursors[ImGuiMouseCursor_COUNT]{};
    Uint64      lastTime = 0;
};

static SDL3Data* s_data = nullptr;

namespace Funkin::ImGui_SDL3 {
    bool init(SDL_Window* window) {
        ImGuiIO& io = ImGui::GetIO();
        io.BackendPlatformName = "imgui_impl_sdl3";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        s_data = new SDL3Data();
        s_data->window = window;

        s_data->cursors[ImGuiMouseCursor_Arrow]      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        s_data->cursors[ImGuiMouseCursor_TextInput]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
        s_data->cursors[ImGuiMouseCursor_ResizeAll]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
        s_data->cursors[ImGuiMouseCursor_ResizeNS]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
        s_data->cursors[ImGuiMouseCursor_ResizeEW]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
        s_data->cursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
        s_data->cursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
        s_data->cursors[ImGuiMouseCursor_Hand]       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
        s_data->cursors[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);

        io.SetClipboardTextFn = [](void*, const char* text) { SDL_SetClipboardText(text); };
        io.GetClipboardTextFn = [](void*) -> const char* { return SDL_GetClipboardText(); };

        s_data->lastTime = SDL_GetTicksNS();
        return true;
    }

    void shutdown() {
        SDL_StopTextInput(s_data->window);
        for (auto& cursor : s_data->cursors)
            if (cursor) SDL_DestroyCursor(cursor);
        delete s_data;
        s_data = nullptr;
    }

    void newFrame() {
        ImGuiIO& io = ImGui::GetIO();

        int w, h;
        SDL_GetWindowSize(s_data->window, &w, &h);
        io.DisplaySize = ImVec2((float)w, (float)h);

        int dw, dh;
        SDL_GetWindowSizeInPixels(s_data->window, &dw, &dh);
        if (w > 0 && h > 0)
            io.DisplayFramebufferScale = ImVec2((float)dw / w, (float)dh / h);
        
        Uint64 now     = SDL_GetTicksNS();
        io.DeltaTime   = (float)((double)(now - s_data->lastTime) / 1e9);
        if (io.DeltaTime <= 0.0f) io.DeltaTime = 1.0f / 60.0f;
        s_data->lastTime = now;

        if (io.WantTextInput) {
            SDL_StartTextInput(s_data->window);
        } else {
            SDL_StopTextInput(s_data->window);
        }

        if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) {
            ImGuiMouseCursor cur = ImGui::GetMouseCursor();
            if (io.MouseDrawCursor || cur == ImGuiMouseCursor_None) {
                SDL_HideCursor();
            } else {
                SDL_Cursor* c = s_data->cursors[cur] 
                    ? s_data->cursors[cur] 
                    : s_data->cursors[ImGuiMouseCursor_Arrow];
                SDL_SetCursor(c);
                SDL_ShowCursor();
            }
        }
    }

    bool processEvent(const SDL_Event& event) {
        ImGuiIO& io = ImGui::GetIO();

        switch (event.type) {
            case SDL_EVENT_MOUSE_MOTION:
                io.AddMousePosEvent(event.motion.x, event.motion.y);
                return true;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                int btn = -1;
                if (event.button.button == SDL_BUTTON_LEFT)   btn = 0;
                if (event.button.button == SDL_BUTTON_RIGHT)  btn = 1;
                if (event.button.button == SDL_BUTTON_MIDDLE) btn = 2;
                if (btn != -1)
                    io.AddMouseButtonEvent(btn, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                return true;
            }

            case SDL_EVENT_MOUSE_WHEEL:
                io.AddMouseWheelEvent(event.wheel.x, event.wheel.y);
                return true;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                ImGuiKey key = [](SDL_Keycode k) -> ImGuiKey {
                    switch (k) {
                        case SDLK_TAB:        return ImGuiKey_Tab;
                        case SDLK_LEFT:       return ImGuiKey_LeftArrow;
                        case SDLK_RIGHT:      return ImGuiKey_RightArrow;
                        case SDLK_UP:         return ImGuiKey_UpArrow;
                        case SDLK_DOWN:       return ImGuiKey_DownArrow;
                        case SDLK_HOME:       return ImGuiKey_Home;
                        case SDLK_END:        return ImGuiKey_End;
                        case SDLK_INSERT:     return ImGuiKey_Insert;
                        case SDLK_DELETE:     return ImGuiKey_Delete;
                        case SDLK_BACKSPACE:  return ImGuiKey_Backspace;
                        case SDLK_SPACE:      return ImGuiKey_Space;
                        case SDLK_RETURN:     return ImGuiKey_Enter;
                        case SDLK_ESCAPE:     return ImGuiKey_Escape;
                        case SDLK_LCTRL:      return ImGuiKey_LeftCtrl;
                        case SDLK_LSHIFT:     return ImGuiKey_LeftShift;
                        case SDLK_LALT:       return ImGuiKey_LeftAlt;
                        case SDLK_RCTRL:      return ImGuiKey_RightCtrl;
                        case SDLK_RSHIFT:     return ImGuiKey_RightShift;
                        case SDLK_RALT:       return ImGuiKey_RightAlt;
                        case SDLK_A:          return ImGuiKey_A;
                        case SDLK_C:          return ImGuiKey_C;
                        case SDLK_V:          return ImGuiKey_V;
                        case SDLK_X:          return ImGuiKey_X;
                        case SDLK_Y:          return ImGuiKey_Y;
                        case SDLK_Z:          return ImGuiKey_Z;
                        default:              return ImGuiKey_None;
                    }
                }(event.key.key);

                io.AddKeyEvent(ImGuiMod_Ctrl,  (SDL_GetModState() & SDL_KMOD_CTRL)  != 0);
                io.AddKeyEvent(ImGuiMod_Shift, (SDL_GetModState() & SDL_KMOD_SHIFT) != 0);
                io.AddKeyEvent(ImGuiMod_Alt,   (SDL_GetModState() & SDL_KMOD_ALT)   != 0);
                if (key != ImGuiKey_None)
                    io.AddKeyEvent(key, event.type == SDL_EVENT_KEY_DOWN);
                return true;
            }

            case SDL_EVENT_TEXT_INPUT:
                io.AddInputCharactersUTF8(event.text.text);
                return true;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                io.AddFocusEvent(true);
                return true;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                io.AddFocusEvent(false);
                return true;
        }
        
        return false;
    }
}