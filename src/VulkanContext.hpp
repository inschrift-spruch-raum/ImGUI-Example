#pragma once

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "wrapper/Vulkan_wrapper.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "wrapper/ImGUI_wrapper.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <array>
#include <cstdint>
#include <print>
#include <stdexcept>

//#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

#ifdef APP_USE_UNLIMITED_FRAME_RATE
    constexpr bool APP_USE_UNLIMITED_FRAME_RATE_ = true;
#else
    constexpr bool APP_USE_UNLIMITED_FRAME_RATE_ = false;
#endif

// Data
class VulkanContext {
private:
    static inline Vulkan::AllocationCallbacks* allocator = nullptr;
    static inline Vulkan::Instance             instance = Vulkan::NULL_HANDLE;
    static inline Vulkan::PhysicalDevice       physicalDevice = Vulkan::NULL_HANDLE;
    static inline Vulkan::Device               device = Vulkan::NULL_HANDLE;
    static inline std::uint32_t                queueFamily = static_cast<std::uint32_t>(-1);
    static inline Vulkan::Queue                queue = Vulkan::NULL_HANDLE;
    static inline Vulkan::PipelineCache        pipelineCache = Vulkan::NULL_HANDLE;
    static inline Vulkan::DescriptorPool       descriptorPool = Vulkan::NULL_HANDLE;
    static inline std::uint32_t                minImageCount = 2;
    static inline bool                         swapChainRebuild = false;

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    static inline Vulkan::DebugReportCallbackEXT debugReport = Vulkan::NULL_HANDLE;
#endif

public:
    VulkanContext() = delete;
    static Vulkan::AllocationCallbacks*& Allocator() { return allocator; }
    static Vulkan::Instance& Instance() { return instance; }
    static Vulkan::PhysicalDevice& PhysicalDevice() { return physicalDevice; }
    static Vulkan::Device& Device() { return device; }
    static std::uint32_t& QueueFamily() { return queueFamily; }
    static Vulkan::Queue& Queue() { return queue; }
    static Vulkan::PipelineCache& PipelineCache() { return pipelineCache; }
    static Vulkan::DescriptorPool& DescriptorPool() { return descriptorPool; }
        static ImGui_ImplVulkanH_Window& MainWindowData() { 
        static ImGui_ImplVulkanH_Window mainWindowData{};  // Lazy initialization
        return mainWindowData; 
    }
    static std::uint32_t& MinImageCount() { return minImageCount; }
    static bool& SwapChainRebuild() {return swapChainRebuild;}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    static Vulkan::DebugReportCallbackEXT& DebugReport() { return debugReport; }
#endif

    static void SetupVulkan(ImGui::Vector<const char*> instance_extensions);
    static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, Vulkan::SurfaceKHR surface, int width, int height);
    static void CleanupVulkan();
    static void CleanupVulkanWindow();
};

static void check_vk_result(Vulkan::Result err)
{
    if (err == VK_SUCCESS) {
        return;
    }
    std::println(stderr, "[vulkan] Error: Vulkan::Result = {}", static_cast<int>(err));
    if (err < 0) {
        abort();
    }
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR Vulkan::Bool32 VKAPI_CALL debug_report(Vulkan::DebugReportFlagsEXT flags, Vulkan::DebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    std::println(stderr, "[vulkan] Debug report from ObjectType: {}\nMessage: {}\n", static_cast<int>(objectType), pMessage);
    return Vulkan::FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const ImGui::Vector<Vulkan::ExtensionProperties>& properties, const char* extension)
{
    for (const Vulkan::ExtensionProperties& p : properties) {
        if (strcmp(static_cast<const char*>(p.extensionName), extension) == 0) {
            return true;
        }
    }
    return false;
}

inline void VulkanContext::SetupVulkan(ImGui::Vector<const char*> instance_extensions)
{
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        Vulkan::InstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count = 0;
        ImGui::Vector<Vulkan::ExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(static_cast<std::int32_t>(properties_count));
        Vulkan::Result err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= static_cast<std::uint32_t>(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR);
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        constexpr std::array<const char*, 1>layers = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers.data();
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.Size);
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, VulkanContext::Allocator(), &VulkanContext::Instance());
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(VulkanContext::Instance());
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT = std::bit_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(VulkanContext::Instance(), "vkCreateDebugReportCallbackEXT"));
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        Vulkan::DebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = static_cast<std::uint32_t>(VK_DEBUG_REPORT_ERROR_BIT_EXT) | static_cast<std::uint32_t>(VK_DEBUG_REPORT_WARNING_BIT_EXT) | static_cast<std::uint32_t>(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(VulkanContext::Instance(), &debug_report_ci, VulkanContext::Allocator(), &VulkanContext::DebugReport());
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    VulkanContext::PhysicalDevice() = ImGui_ImplVulkanH_SelectPhysicalDevice(VulkanContext::Instance());
    IM_ASSERT(VulkanContext::PhysicalDevice() != Vulkan::NULL_HANDLE);

    // Select graphics queue family
    VulkanContext::QueueFamily() = ImGui_ImplVulkanH_SelectQueueFamilyIndex(VulkanContext::PhysicalDevice());
    IM_ASSERT(VulkanContext::QueueFamily() != static_cast<std::uint32_t>(-1));

    // Create Logical Device (with 1 queue)
    {
        ImGui::Vector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count = 0;
        ImGui::Vector<Vulkan::ExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(VulkanContext::PhysicalDevice(), nullptr, &properties_count, nullptr);
        properties.resize(static_cast<std::int32_t>(properties_count));
        vkEnumerateDeviceExtensionProperties(VulkanContext::PhysicalDevice(), nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const std::array<float, 1> queue_priority = { 1.0F };
        std::array<Vulkan::DeviceQueueCreateInfo, 1> queue_info = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = VulkanContext::QueueFamily();
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority.data();
        Vulkan::DeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info.data();
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.Size);
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        Vulkan::Result err = vkCreateDevice(VulkanContext::PhysicalDevice(), &create_info, VulkanContext::Allocator(), &VulkanContext::Device());
        check_vk_result(err);
        vkGetDeviceQueue(VulkanContext::Device(), VulkanContext::QueueFamily(), 0, &VulkanContext::Queue());
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
    {
        std::array<Vulkan::DescriptorPoolSize, 1> pool_sizes =
        {
            Vulkan::DescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE}
        };
        Vulkan::DescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (Vulkan::DescriptorPoolSize& pool_size : pool_sizes) {
            pool_info.maxSets += pool_size.descriptorCount;
        }
        pool_info.poolSizeCount = pool_sizes.size();
        pool_info.pPoolSizes = pool_sizes.data();
        Vulkan::Result err = vkCreateDescriptorPool(VulkanContext::Device(), &pool_info, VulkanContext::Allocator(), &VulkanContext::DescriptorPool());
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
inline void VulkanContext::SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, Vulkan::SurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    Vulkan::Bool32 res = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(VulkanContext::PhysicalDevice(), VulkanContext::QueueFamily(), wd->Surface, &res);
    if (res != VK_TRUE)
    {
        std::println(stderr, "Error no WSI support on physical device 0");
        throw std::runtime_error("Error no WSI support on physical device 0");
    }

    // Select Surface Format
    const std::array<Vulkan::Format, 4> requestSurfaceImageFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const Vulkan::ColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(VulkanContext::PhysicalDevice(), wd->Surface, requestSurfaceImageFormat.data(), requestSurfaceImageFormat.size(), requestSurfaceColorSpace);

    // Select Present Mode
    constexpr auto present_modes = []() constexpr {
        if constexpr (APP_USE_UNLIMITED_FRAME_RATE_) {
            return std::array{
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_IMMEDIATE_KHR,
                VK_PRESENT_MODE_FIFO_KHR
            };
        } else {
            return std::array{
                VK_PRESENT_MODE_FIFO_KHR
            };
        }
    }();

    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(VulkanContext::PhysicalDevice(), wd->Surface, present_modes.data(), present_modes.size());
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(VulkanContext::MinImageCount() >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(VulkanContext::Instance(), VulkanContext::PhysicalDevice(), VulkanContext::Device(), wd, VulkanContext::QueueFamily(), VulkanContext::Allocator(), width, height, VulkanContext::MinImageCount(), 0);
}

inline void VulkanContext::CleanupVulkan()
{
    vkDestroyDescriptorPool(VulkanContext::Device(), VulkanContext::DescriptorPool(), VulkanContext::Allocator());

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT = std::bit_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(VulkanContext::Instance(), "vkDestroyDebugReportCallbackEXT"));
    f_vkDestroyDebugReportCallbackEXT(VulkanContext::Instance(), VulkanContext::DebugReport(), VulkanContext::Allocator());
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(VulkanContext::Device(), VulkanContext::Allocator());
    vkDestroyInstance(VulkanContext::Instance(), VulkanContext::Allocator());
}

inline void VulkanContext::CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(VulkanContext::Instance(), VulkanContext::Device(), &VulkanContext::MainWindowData(), VulkanContext::Allocator());
}
