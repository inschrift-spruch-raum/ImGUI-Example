#pragma once

#include "vk.hpp"
#include <cstdint>

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    Vulkan::Semaphore image_acquired_semaphore  = wd->FrameSemaphores[static_cast<std::int32_t>(wd->SemaphoreIndex)].ImageAcquiredSemaphore;
    Vulkan::Semaphore render_complete_semaphore = wd->FrameSemaphores[static_cast<std::int32_t>(wd->SemaphoreIndex)].RenderCompleteSemaphore;
    Vulkan::Result err = vkAcquireNextImageKHR(vk::Device(), wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        vk::SwapChainRebuild() = true;
    }
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }
    if (err != VK_SUBOPTIMAL_KHR) {
        check_vk_result(err);
    }

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[static_cast<std::int32_t>(wd->FrameIndex)];
    {
        err = vkWaitForFences(vk::Device(), 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(vk::Device(), 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(vk::Device(), fd->CommandPool, 0);
        check_vk_result(err);
        Vulkan::CommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= static_cast<std::uint32_t>(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        Vulkan::RenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        Vulkan::PipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        Vulkan::SubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(vk::Queue(), 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (vk::SwapChainRebuild()) {
        return;
    }
    Vulkan::Semaphore render_complete_semaphore = wd->FrameSemaphores[static_cast<std::int32_t>(wd->SemaphoreIndex)].RenderCompleteSemaphore;
    Vulkan::PresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    Vulkan::Result err = vkQueuePresentKHR(vk::Queue(), &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        vk::SwapChainRebuild() = true;
    }
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }
    if (err != VK_SUBOPTIMAL_KHR) {
        check_vk_result(err);
    }
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}
