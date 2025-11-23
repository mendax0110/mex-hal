#ifndef MEX_HAL_LOCKER_H
#define MEX_HAL_LOCKER_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <thread>

#define DROP_LOCKER(locker, duration_ms)                \
    do {                                                \
        locker.unlock();                                \
        std::this_thread::sleep_for(                    \
            std::chrono::milliseconds(duration_ms));    \
        locker.lock();                                  \
    } while (0)

#define SCOPED_UNLOCK(locker)                                               \
    std::unique_lock<std::mutex> _scopedUnlock(locker, std::adopt_lock);    \
    _scopedUnlock.unlock();                                                 \
    auto _scopedUnlockGuard = [&]() { _scopedUnlock.lock(); }

#define TRY_LOCK_FOR(locker, duration_ms, success)                              \
    bool success = false;                                                       \
    {                                                                           \
        std::unique_lock<std::mutex> _lock(locker, std::defer_lock);            \
        success = _lock.try_lock_for(std::chrono::milliseconds(duration_ms));   \
    }

#define WAIT_FOR(condVar, locker, duration_ms, notified)                                                            \
    bool notified = false;                                                                                          \
    {                                                                                                               \
        std::unique_lock<std::mutex> _lock(locker);                                                                 \
        notified = condVar.wait_for(_lock, std::chrono::milliseconds(duration_ms)) == std::cv_status::no_timeout;   \
    }

#endif // MEX_HAL_LOCKER_H
