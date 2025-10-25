#include "../include/hal/hal_state_engine.h"
#include "../src/device_config/device_config.h"
#include "../src/sys_config/sys_config.h"
#include "../include/hal/resource_visualizer.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <limits>
#include <atomic>
#include <termios.h>
#include <unistd.h>

using namespace mex_hal;

static bool running = true;

void handleSignal(int)
{
    running = false;
}

void printMenu()
{
    std::cout << "\n===== MEX-HAL Interactive Menu =====\n";
    std::cout << "1. Show system configuration\n";
    std::cout << "2. Show device information\n";
    std::cout << "3. Show HAL state\n";
    std::cout << "4. Toggle real-time policy (FIFO/RR/NONE)\n";
    std::cout << "5. Show resource usage (live)\n";
    std::cout << "6. Show resource graph\n";
    std::cout << "7. Exit\n";
    std::cout << "Select an option: ";
}

void setTerminalRawMode(const bool enable)
{
    static termios oldt;
    termios newt{};
    if (enable)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

void liveResourceView(ResourceVisualizer& visualizer, int intervalMs = 500)
{
    std::atomic<bool> liveRunning{true};
    setTerminalRawMode(true);

    std::cout << "\n=== Live Resource View ===\n";
    std::cout << "Press 'q' to return to menu\n";

    std::thread updateThread([&]()
    {
        while (liveRunning)
        {
            visualizer.gatherResourceData();
            visualizer.buildResourceGraph();
            std::cout << "\033[2J\033[H"; // clear screen
            visualizer.printResourceUsage();
            std::cout << "\nPress 'q' to return to menu\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    });

    char c{};
    while (liveRunning && read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == 'q' || c == 'Q')
            liveRunning = false;
    }

    updateThread.join();
    setTerminalRawMode(false);
}

int main()
{
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    ResourceVisualizer visualizer;
    visualizer.startLiveUpdate(500);

    auto status = SystemConfig::check();
    auto& conf = DeviceConfig::getInstance();

    auto& engine = HALStateEngine::getInstance();
    engine.start();
    std::cout << "[Main] HAL State Engine started.\n";

    const auto hal = createHAL(HALType::LINUX);

    while (running)
    {
        printMenu();
        int choice{};
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice)
        {
            case 1:
                status = SystemConfig::check();
                SystemConfig::printReport(status);
                break;
            case 2:
                conf.printDeviceInfos();
                break;
            case 3:
            {
                const auto state = engine.getState();
                std::cout << "HAL State: " << (state == HALState::RUNNING ? "RUNNING" : "STOPPED") << "\n";

                const auto rtState = hal->getRealtimeState();
                std::cout << "Realtime state: ";
                switch (rtState)
                {
                    case RealTimeState::RUNNING:     std::cout << "RUNNING\n"; break;
                    case RealTimeState::NOT_RUNNING: std::cout << "NOT RUNNING\n"; break;
                    case RealTimeState::ERROR:       std::cout << "ERROR\n"; break;
                }
                break;
            }
            case 4:
            {
                std::cout << "Set Real-time policy (0=NONE, 1=FIFO, 2=RR): ";
                int pol{};
                if (std::cin >> pol)
                {
                    auto p = RealTimePolicy::INVALID;
                    switch (pol)
                    {
                        case 0: p = hal->setRealTimePolicy(RealTimePolicy::NONE); break;
                        case 1: p = hal->setRealTimePolicy(RealTimePolicy::FIFO); break;
                        case 2: p = hal->setRealTimePolicy(RealTimePolicy::RR); break;
                        default: std::cout << "Invalid option\n"; break;
                    }

                    if (pol >= 0 && p != RealTimePolicy::INVALID)
                    {
                        std::cout << "Realtime policy set to " << static_cast<int>(p) << "\n";
                    }
                }
                break;
            }
            case 5:
                liveResourceView(visualizer, 500);
                break;
            case 6:
                visualizer.printResourceGraph();
                break;
            case 7:
                running = false;
                break;
            default:
                std::cout << "Unknown option\n";
                break;
        }
    }

    visualizer.stopLiveUpdate();
    engine.stop();
    std::cout << "[Main] HAL State Engine stopped. Exiting.\n";

    return 0;
}
