#include <iostream>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <cstdlib>

#include "vulkanCore.hpp"
#include "commandArgs.hpp"
#include "mode.hpp"
#include "playMode.hpp"

static const std::string helpMessage = " To run application, arguments must formatted as follows\n : \
--REQUIRED---------------------------------------------------------------------------------------\n \
[] --scene {scene.s72} : REQUIRED, name of .s72 scene file to run.\n \
--OPTIONAL---------------------------------------------------------------------------------------\n \
[] --debug-level {l} : level at which to do debug checks, integer from 0-3 inclusive \n \
    0 - do no debugging check (same as running with NDEBUG compiler flag) \n \
    1 - do critical debugging checks (ie checking for validation layer support) \n \
    2 - do level 1 checks + validating data strucutes \n \
    3 - do level 2 checks + sanity check test cases \n \
[] --print-debug-output : whether to print the output of debugging checks \n \
[] --camera {camera} : name of camera in scene.\n \
[] --physical-device {device} : physical device whose VkPhysicalDeviceProperties::deviceName matches name.\n \
		run .exe with argument --list-physical-devices to see all device names available to you.\n \
[] --resolution {w} {h}, width and height of drawing canvas in pixels.\n \
[] --frustum-culling : enable frustum (outside of camera frustum) culling \n \
[] --occlusion-culling : enable occlusion (covered by other objects) culling \n \
[] --swapchain-mode {mode} where mode is one of \n \
       fifo (DEFAULT), gauranteed to be available  \n \
       immediate  \n \
       fifo-relaxed \n \
       mailbox \n \
[] --force-show-fps : show fps counter in window even if not in debug mode \n \
[] --multisampling {s} : s is int number of samples to do antialiasing in rasterization stage, one of \n \
    1 (DEFAULT), 2, 4, 8, 16, 32, 64 \n \
[] --headless {events}, path to UTF-8-encoded text file to perform application in headless mode.\n \
		Must supply drawing_size in this case \n \
[] --stripify : stripify mesh of scene into triangle strips with tunneling algirthm in stripify.cpp \n \
[] --cluster : cluser mesh into mesh lets of size cluster_size, with default of 64 \n \
           if culling is activated, culling isbased off meshlet bounding boxes and not mesh boinding boxes \n \
[] --cluster-size {s} : number of vertices in each triangle strip cluster size \n";


int main(int argc, char* argv[]) {
    try {
        parseCommandLine(argc, argv);
    } catch (const std::exception& e){
        (void)e;
        std::cerr << helpMessage << std::endl;
        return EXIT_FAILURE;
    }

    {
        App app = App(commandLineParameters);
        if (app.DEBUG && app.globalParameters.DEBUG_LEVEL >= ALL_OUTPUT && app.globalParameters.PRINT_DEBUG) printParameters();

        Mode::set_current(std::make_shared<PlayMode>());

        try {
            app.initWindow();
            app.initVulkan();
            app.initProgram(*Mode::current);
            while (!glfwWindowShouldClose(app.window)) {
                app.FRAME++;
                glfwPollEvents();

                //process user input
                while (!Input::inputEventsQueue.empty()) {
                    if(Mode::current) Mode::current->handleEvent(Input::inputEventsQueue.front(), {app.WIDTH, app.HEIGHT});
                    Input::inputEventsQueue.pop();
                }

                { 

                    static auto startTime = std::chrono::high_resolution_clock::now();
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    static auto previousTime = currentTime;
                    float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
                    float totalTime = std::chrono::duration<float>(currentTime - startTime).count();
                    previousTime = currentTime;

                    //if frames are taking a very long time to process,
                    //lag to avoid spiral of death:
                    deltaTime = std::min(0.1f, deltaTime);

                    Mode::current->update(deltaTime, totalTime);
                    if (!Mode::current) break;
                }

                std::pair<VkCommandBuffer, uint32_t> beginInfo = app.beginFrame();
                //if beginning return null,(ie if window resized and swachain needed to be resize), 
                //beginInfo is invalid
                if (beginInfo.first == nullptr || beginInfo.second == -1U) continue;
                Mode::current->draw(app, beginInfo.first, beginInfo.second);
                app.endFrame(beginInfo.first, beginInfo.second); //TODO eventually bring into Mode
            }
            app.cleanupProgram();
            app.cleanupVulkan();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}