#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan {
    using PipelineCache = VkPipelineCache;
    using SurfaceKHR = VkSurfaceKHR;
    using Result = VkResult;
    using DebugReportCallbackEXT = VkDebugReportCallbackEXT;
    using AllocationCallbacks = VkAllocationCallbacks;
    using Instance = VkInstance;
    using PhysicalDevice = VkPhysicalDevice;
    using Device = VkDevice;
    using Queue = VkQueue;
    using DescriptorPool = VkDescriptorPool;
    using DebugReportObjectTypeEXT = VkDebugReportObjectTypeEXT;
    using ExtensionProperties = VkExtensionProperties;
    using InstanceCreateInfo = VkInstanceCreateInfo;
    using DebugReportCallbackCreateInfoEXT = VkDebugReportCallbackCreateInfoEXT;
    using Bool32 = VkBool32;
    using DebugReportFlagsEXT = VkDebugReportFlagsEXT;
    using DeviceQueueCreateInfo = VkDeviceQueueCreateInfo;
    using DeviceCreateInfo = VkDeviceCreateInfo;
    using DescriptorPoolSize = VkDescriptorPoolSize;
    using DescriptorPoolCreateInfo = VkDescriptorPoolCreateInfo;
    using CommandBufferBeginInfo = VkCommandBufferBeginInfo;
    using RenderPassBeginInfo = VkRenderPassBeginInfo;
    using SubmitInfo = VkSubmitInfo;
    using PipelineStageFlags = VkPipelineStageFlags;
    using PresentInfoKHR = VkPresentInfoKHR;
    using Format = VkFormat;
    using ColorSpaceKHR = VkColorSpaceKHR;
    using PresentModeKHR = VkPresentModeKHR;
    using Semaphore = VkSemaphore;
    using SurfaceFormatKHR = VkSurfaceFormatKHR;
    using CommandPool   = VkCommandPool  ;
    using CommandBuffer = VkCommandBuffer;
    using Fence         = VkFence        ;
    using Image         = VkImage        ;
    using ImageView     = VkImageView    ;
    using Framebuffer   = VkFramebuffer  ;
    using SwapchainKHR = VkSwapchainKHR;
    using RenderPass = VkRenderPass;
    using ClearValue = VkClearValue;
    using ImageUsageFlags = VkImageUsageFlags;

    static constexpr auto NULL_HANDLE = VK_NULL_HANDLE;
    static constexpr auto FALSE = VK_FALSE;
}  // namespace Vulkan
