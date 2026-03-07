#include "timer_mock.h"

using namespace mex_hal;
using namespace std::chrono;

TimerMock::~TimerMock()
{
    stop();
}

bool TimerMock::init(const TimerMode timerMode)
{
    mode_ = timerMode;
    return true;
}

void TimerMock::timerLoop()
{
    startTime_ = steady_clock::now();

    while (!shouldStop_.load())
    {
        std::this_thread::sleep_for(microseconds(intervalUs_));

        if (!shouldStop_.load())
        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (callback_)
            {
                callback_();
            }
        }

        if (mode_ == TimerMode::ONE_SHOT)
        {
            break;
        }
    }

    running_.store(false);
}

bool TimerMock::start(const uint64_t interval, const TimerCallback cb)
{
    if (running_.load()) return false;

    intervalUs_ = interval;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = cb;
    }

    shouldStop_.store(false);
    running_.store(true);
    timerThread_ = std::thread(&TimerMock::timerLoop, this);
    return true;
}

bool TimerMock::stop()
{
    shouldStop_.store(true);
    if (!timerThread_.joinable()) return false;
    timerThread_.join();
    running_.store(false);
    return true;
}

bool TimerMock::reset()
{
    if (running_.load()) stop();
    startTime_ = steady_clock::now();
    return true;
}

bool TimerMock::setInterval(const uint64_t interval)
{
    if (running_.load()) return false;
    intervalUs_ = interval;
    return true;
}

uint64_t TimerMock::getInterval() const
{
    return intervalUs_;
}

bool TimerMock::isRunning() const
{
    return running_.load();
}

uint64_t TimerMock::getElapsedUs() const
{
    const auto now = steady_clock::now();
    return static_cast<uint64_t>(duration_cast<microseconds>(now - startTime_).count());
}

uint64_t TimerMock::getCurrentTimeUs() const
{
    auto now = steady_clock::now();
    return static_cast<uint64_t>(duration_cast<microseconds>(now.time_since_epoch()).count());
}
