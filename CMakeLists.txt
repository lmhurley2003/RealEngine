cmake_minimum_required(VERSION 3.11)
project(real)

set(CMAKE_CXX_STANDARD 20)

if(DEFINED BUILD_SHADERS AND BUILD_SHADERS)
    message(STATUS "Including CMakeBuildShaders.cmake")
    include(CMakeBuildShaders.cmake)
    compile_shaders()
    create_resources(shaders/compiled shaders/embedded)
    return()
endif()

include(CMakeGlobalMacros.cmake)

find_package(Vulkan REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/headers
                    ${PROJECT_SOURCE_DIR}/source
                    ${PROJECT_SOURCE_DIR}/shaders
                    ${PROJECT_SOURCE_DIR}/shaders/embedded
                    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glm"
                    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glfw-3.3.9.bin.WIN64\\Include"
                    "C:\\VulkanSDK\\1.4.313.2\\Include"
                    "C:\\Users\\lucas\\source\\repos\\stb"
)

link_libraries (
    ${Vulkan_LIBRARIES}
    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glfw-3.3.9.bin.WIN64\\lib-vc2022\\glfw3.lib"
)

set(real_headers
    headers/vulkanCore.hpp
    headers/vertexIndex.hpp
    headers/commandArgs.hpp
    headers/animation.hpp
    headers/parameters.hpp
    headers/material.hpp
    headers/mesh.hpp
    headers/light.hpp
    headers/camera.hpp
    headers/scene.hpp
    headers/jsonParsing.hpp
    headers/input.hpp
    headers/mode.hpp
    headers/playMode.hpp
    headers/entityComponent.hpp
    headers/utils.hpp
)

file(GLOB SOURCE_EMBEDDED_SHADERS "shaders/embedded/*.cpp")
set(real_source
    source/commandArgs.cpp
    source/jsonParsing.cpp
    source/entityComponent.cpp
    source/input.cpp
    source/mesh.cpp
    source/camera.cpp
    source/scene.cpp
    source/mode.cpp
    source/playMode.cpp
    source/utils.cpp
    source/vertexIndex.cpp
    source/vulkanMemory.cpp
    source/vulkanCore.cpp
    ${SOURCE_EMBEDDED_SHADERS}
)

# Compile core real library
add_library(real_core STATIC
    ${real_headers}
    ${real_source}
)

add_executable(real source/main.cpp)

target_link_libraries(real real_core)

set_property(TARGET real PROPERTY CXX_STANDARD 20)
set_property(TARGET real PROPERTY CXX_STANDARD_REQUIRED ON)



