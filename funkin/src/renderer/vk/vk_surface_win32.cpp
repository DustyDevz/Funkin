// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#ifdef _WIN32
    #include <renderer/renderer_common.hpp>
    #include <platform/window/window_win32.hpp>
    #include "vk_surface.hpp"
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <vulkan/vulkan_win32.h>

    namespace Funkin::Renderer::VK {
        void VK_Surface::init(VkInstance instance) {
            auto& win = Funkin::Platform::Window_Win32::get();

            VkWin32SurfaceCreateInfoKHR ci{};
            ci.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            ci.hwnd      = win.hwnd();
            ci.hinstance = win.hinstance();

            vkCheck(vkCreateWin32SurfaceKHR(instance, &ci, nullptr, &m_surface),
                "Failed to create Win32 surface");
        }

        void VK_Surface::shutdown(VkInstance instance) {
            if (m_surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(instance, m_surface, nullptr);
                m_surface = VK_NULL_HANDLE;
            }
        }
    }
#endif