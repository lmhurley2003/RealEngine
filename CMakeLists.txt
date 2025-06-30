cmake_minimum_required(VERSION 3.5)
project(real)

set(CMAKE_CXX_STANDARD 20)

function(create_resources dir)
    # Collect input files
    file(GLOB bins ${dir}/*)
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})

        # get rid of _spv extension
        string(LENGTH ${filename} STRING_LENGTH)
        math(EXPR NEW_LENGTH "${STRING_LENGTH} - 4")
        string(SUBSTRING ${filename} 0 "${NEW_LENGTH}" trimmedFileName)

        file(WRITE "shaders/embedded/${trimmedFileName}.cpp" "")
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND "shaders/embedded/${trimmedFileName}.cpp" "const unsigned char ${trimmedFileName}[] = {${filedata}};\nconst unsigned ${trimmedFileName}Size = sizeof(${trimmedFileName});\n")
    endforeach()
endfunction()

find_package(Vulkan REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/source
                    ${PROJECT_SOURCE_DIR}/headers
                    ${PROJECT_SOURCE_DIR}/shaders
                    ${PROJECT_SOURCE_DIR}/shaders/embedded
                    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glm"
                    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glfw-3.3.9.bin.WIN64\\Include"
                    "C:\\VulkanSDK\\1.4.313.2\\Include"
)

link_libraries (
    ${Vulkan_LIBRARIES}
    "C:\\Users\\lucas\\OneDrive\\Documents\\Visual Studio 2022\\Libraries\\glfw-3.3.9.bin.WIN64\\lib-vc2022\\glfw3.lib"
)

#comment out, takes long time
#create_resources(shaders/compiled source/embeddedShaders.cpp)

set(real_headers
    headers/commandArgs.hpp
    headers/program.hpp
    headers/scene.hpp
    headers/utils.hpp
    headers/vulkanConfiguration.hpp
)

file(GLOB SOURCE_FILES "shaders/embedded/*.cpp")
set(real_source
    source/commandArgs.cpp
    source/program.cpp
    source/scene.cpp
    source/utils.cpp
    source/vulkanConfiguration.cpp
    ${SOURCE_FILES}
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



