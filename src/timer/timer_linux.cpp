#include "timer_linux.h"

using namespace mex_hal;
using namespace std::chrono;

TimerLinux::~TimerLinux()
{
    stop();
}

bool TimerLinux::init(const TimerMode timerMode)
{
    mode = timerMode;
    return true;
}

void TimerLinux::timerLoop()
{
    startTime = steady_clock::now();
    
    while (!shouldStop.load())
    {
        auto nextTick = startTime + microseconds(intervalUs);
        
        if (mode == TimerMode::PERIODIC)
        {
            std::this_thread::sleep_until(nextTick);
        }
        else
        {
            std::this_thread::sleep_for(microseconds(intervalUs));
        }
        
        if (!shouldStop.load())
        {
            std::lock_guard<std::mutex> lock(callbackMutex);
            if (callback)
            {
                callback();
            }
        }
        
        if (mode == TimerMode::ONE_SHOT)
        {
            break;
        }
        
        if (mode == TimerMode::PERIODIC)
        {
            startTime = nextTick;
        }
    }
    
    running.store(false);
}

bool TimerLinux::start(const uint64_t interval, const TimerCallback cb)
{
    if (running.load())
    {
        return false;
    }
    
    intervalUs = interval;
    
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        callback = cb;
    }
    
    shouldStop.store(false);
    running.store(true);
    
    timerThread = std::thread(&TimerLinux::timerLoop, this);
    
    return true;
}

bool TimerLinux::stop()
{
    shouldStop.store(true);

    if (!timerThread.joinable())
        return false;

    timerThread.join();
    running.store(false);
    return true;
}

bool TimerLinux::reset()
{
    if (running.load())
    {
        stop();
    }
    
    startTime = steady_clock::now();
    return true;
}

bool TimerLinux::setInterval(uint64_t interval)
{
    if (running.load())
    {
        return false;
    }
    
    intervalUs = interval;
    return true;
}

uint64_t TimerLinux::getInterval() const
{
    return intervalUs;
}

bool TimerLinux::isRunning() const
{
    return running.load();
}

uint64_t TimerLinux::getElapsedUs() const
{
    auto now = steady_clock::now();
    auto elapsed = duration_cast<microseconds>(now - startTime);
    return static_cast<uint64_t>(elapsed.count());
}

uint64_t TimerLinux::getCurrentTimeUs() const
{
    auto now = steady_clock::now();
    auto epoch = now.time_since_epoch();
    return static_cast<uint64_t>(duration_cast<microseconds>(epoch).count());
}
