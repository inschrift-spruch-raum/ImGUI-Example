cmake_minimum_required(VERSION 4.0)

project(ImGUI-Example VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MSYS2_Clang "Compile using Clang in MSYS2" OFF)
option(Vulkan_SDL3 "Using SDL3 provided by the Vulkan library" OFF)
option(SDL3_static "Link SDL3-static in the release build." OFF)

find_package(Vulkan REQUIRED)

get_filename_component(Vulkan_Libs_DIR ${Vulkan_LIBRARIES} DIRECTORY)
get_filename_component(Vulkan_DIR ${Vulkan_INCLUDE_DIR} DIRECTORY)
get_filename_component(Vulkan_Bin_DIR ${Vulkan_GLSLC_EXECUTABLE} DIRECTORY)

if(Vulkan_SDL3)
    set(SDL3_DIR "${Vulkan_DIR}/cmake")
endif()

find_package(SDL3 REQUIRED CONFIG)

include_directories(
    "external/ImGUI"
    "external/ImGUI/backends"
)
set(ImGUI
    "external/ImGUI/imgui.cpp"
    "external/ImGUI/imgui_draw.cpp"
    "external/ImGUI/imgui_demo.cpp"
    "external/ImGUI/imgui_tables.cpp"
    "external/ImGUI/imgui_widgets.cpp"
    "external/ImGUI/backends/imgui_impl_sdl3.cpp"
    "external/ImGUI/backends/imgui_impl_vulkan.cpp"
)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

add_executable(ImGUI-Example ${ImGUI} "src/main.cpp")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ImGUI-Example PROPERTY WIN32_EXECUTABLE TRUE)
endif()

if(MSYS2_Clang)
    target_link_libraries(ImGUI-Example PRIVATE 
        -static
    )
endif()

target_link_libraries(ImGUI-Example PRIVATE Vulkan::Vulkan)

if(WIN32 AND SDL3_static)
    target_link_libraries(ImGUI-Example PRIVATE
        $<$<CONFIG:Debug>:SDL3::SDL3>
        $<$<CONFIG:Release>:SDL3::SDL3-static>
    )
else()
    target_link_libraries(ImGUI-Example PRIVATE SDL3::SDL3)
endif()

install(TARGETS ImGUI-Example DESTINATION installed)
install(FILES $<TARGET_RUNTIME_DLLS:ImGUI-Example>
        DESTINATION installed)

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "ImGUI-Example")
set(CPACK_PACKAGE_VERSION "0.0.0")
include(CPack)
