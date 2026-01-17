// Dear ImGui: standalone example application for SDL3 + Vulkan

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "global.hpp"

#include "frame.hpp"
#include "imgui_impl_sdl3.h"
#include "VulkanContext.hpp"
#include "wrapper/ImGUI_wrapper.hpp"
#include "wrapper/Vulkan_wrapper.hpp"
#include <cstdint>
#include <format>
#include <print>
#include <span>
#include <string>

// Main code
int main(int argc, char *argv[])
{
    (void)argc, (void)argv;
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        std::println("Error: SDL_Init(): {}", SDL_GetError());
        return 1;
    }

    // Create window with Vulkan graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL3+Vulkan example", static_cast<int>(1280 * main_scale), static_cast<int>(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
        return 1;
    }

    ImGui::Vector<const char*> extensions;
    {
        uint32_t sdl_extensions_count = 0;
        auto sdl_extensions = std::span<const char* const>{SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count), sdl_extensions_count};
        for (uint32_t n = 0; n < sdl_extensions_count; n++) {
            extensions.push_back(sdl_extensions[n]);
        }
    }
    VulkanContext::SetupVulkan(extensions);

    // Create Window Surface
    Vulkan::SurfaceKHR surface = nullptr;
    if (static_cast<int>(SDL_Vulkan_CreateSurface(window, VulkanContext::Instance(), VulkanContext::Allocator(), &surface)) == 0)
    {
        std::println("Failed to create Vulkan surface.");
        return 1;
    }

    // Create Framebuffers
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &VulkanContext::MainWindowData();
    VulkanContext::SetupVulkanWindow(wd, surface, w, h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    uint32_t ConfigFlags = io.ConfigFlags;
    ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_NavEnableKeyboard);     // Enable Keyboard Controls
    ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_NavEnableGamepad);      // Enable Gamepad Controls
    ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_DockingEnable);         // Enable Docking
    ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_ViewportsEnable);       // Enable Multi-Viewport / Platform Windows
    //ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_ViewportsNoTaskBarIcons);
    //ConfigFlags |= static_cast<uint32_t>(ImGuiConfigFlags_ViewportsNoMerge);
    io.ConfigFlags = static_cast<int32_t>(ConfigFlags);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if ((static_cast<uint32_t>(io.ConfigFlags) & static_cast<uint32_t>(ImGuiConfigFlags_ViewportsEnable)) != 0)
    {
        style.WindowRounding = 0.0F;
        style.Colors[ImGuiCol_WindowBg].w = 1.0F;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = VulkanContext::Instance();
    init_info.PhysicalDevice = VulkanContext::PhysicalDevice();
    init_info.Device = VulkanContext::Device();
    init_info.QueueFamily = VulkanContext::QueueFamily();
    init_info.Queue = VulkanContext::Queue();
    init_info.PipelineCache = VulkanContext::PipelineCache();
    init_info.DescriptorPool = VulkanContext::DescriptorPool();
    init_info.MinImageCount = VulkanContext::MinImageCount();
    init_info.ImageCount = wd->ImageCount;
    init_info.Allocator = VulkanContext::Allocator();
    init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImGui::Vec4 clear_color = ImGui::Vec4(0.45F, 0.55F, 0.60F, 1.00F);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0U)
        {
            SDL_Delay(10);
            continue;
        }

        // Resize swap chain?
        int fb_width = 0;
        int fb_height = 0;
        SDL_GetWindowSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (VulkanContext::SwapChainRebuild() || VulkanContext::MainWindowData().Width != fb_width || VulkanContext::MainWindowData().Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(VulkanContext::MinImageCount());
            ImGui_ImplVulkanH_CreateOrResizeWindow(VulkanContext::Instance(), VulkanContext::PhysicalDevice(), VulkanContext::Device(), wd, VulkanContext::QueueFamily(), VulkanContext::Allocator(), fb_width, fb_height, VulkanContext::MinImageCount(), 0);
            VulkanContext::MainWindowData().FrameIndex = 0;
            VulkanContext::MainWindowData().SemaphoreIndex = 0;
            VulkanContext::SwapChainRebuild() = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0F;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text(std::string("This is some useful text."));               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0F, 1.0F);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", std::bit_cast<float*>(&clear_color)); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) {                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text(std::format("counter = {}", counter));

            ImGui::Text(std::format("Application average {:.3f} ms/frame ({:.1f} FPS)", 1000.0F / io.Framerate, io.Framerate));
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text(std::format("Hello from another window!"));
            if (ImGui::Button("Close Me")) {
                show_another_window = false;
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0F || main_draw_data->DisplaySize.y <= 0.0F);
        wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        wd->ClearValue.color.float32[3] = clear_color.w;
        if (!main_is_minimized) {
            FrameRender(wd, main_draw_data);
        }

        // Update and Render additional Platform Windows
        if ((static_cast<uint32_t>(io.ConfigFlags) & static_cast<uint32_t>(ImGuiConfigFlags_ViewportsEnable)) != 0)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present Main Platform Window
        if (!main_is_minimized) {
            FramePresent(wd);
        }
    }

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    Vulkan::Result err = vkDeviceWaitIdle(VulkanContext::Device());
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    VulkanContext::CleanupVulkanWindow();
    VulkanContext::CleanupVulkan();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
