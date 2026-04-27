#include <vulkan/vulkan.h>
#include <cstdint>

namespace Funkin::Renderer::VK {
    struct QueueFamilies {
        uint32_t graphics = 0;
        uint32_t present  = 0;
    };

    class VK_Device {
    public:
        void init(VkInstance instance, VkSurfaceKHR surface);
        void shutdown();

        VkPhysicalDevice physical() const { return m_physical; }
        VkDevice         logical()  const { return m_device; }
        VkQueue          graphics() const { return m_graphicsQueue; }
        VkQueue          present()  const { return m_presentQueue; }
        QueueFamilies    families() const { return m_families; }

    private:
        void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        void createLogicalDevice(VkSurfaceKHR surface);

        VkPhysicalDevice m_physical      = VK_NULL_HANDLE;
        VkDevice         m_device        = VK_NULL_HANDLE;
        VkQueue          m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue          m_presentQueue  = VK_NULL_HANDLE;
        QueueFamilies    m_families{};
    };
}