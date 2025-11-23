#ifndef MEX_HAL_FILE_DESCRIPTOR_H
#define MEX_HAL_FILE_DESCRIPTOR_H

#include <unistd.h>
#include <atomic>
#include <memory>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /**
     * @brief Thread-safe RAII wrapper for file descriptors
     * 
     * Provides automatic cleanup and thread-safe access to file descriptors.
     * Ensures proper resource management and prevents descriptor leaks.
     */
    class FileDescriptor
    {
    public:
        /**
         * @brief Construct with invalid file descriptor
         */
        FileDescriptor() : fd_(-1) {}

        /**
         * @brief Construct with existing file descriptor
         * @param fd File descriptor to wrap (takes ownership)
         */
        explicit FileDescriptor(const int fd) : fd_(fd) {}

        /**
         * @brief Destructor - automatically closes file descriptor
         */
        ~FileDescriptor()
        {
            close();
        }

        // Prevent copying
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(const FileDescriptor&) = delete;

        /**
         * @brief Move constructor
         * @param other The other FileDescriptor to move from
         */
        FileDescriptor(FileDescriptor&& other) noexcept
        {
            fd_.store(other.fd_.exchange(-1, std::memory_order_acq_rel), std::memory_order_release);
        }

        /**
         * @brief Move assignment operator
         * @param other The other FileDescriptor to move from
         * @return Reference to this FileDescriptor
         */
        FileDescriptor& operator=(FileDescriptor&& other) noexcept
        {
            if (this != &other)
            {
                close();
                fd_.store(other.fd_.exchange(-1, std::memory_order_acq_rel), std::memory_order_release);
            }
            return *this;
        }

        /**
         * @brief Get the file descriptor value
         * @return File descriptor value (-1 if invalid)
         */
        [[nodiscard]] int get() const
        {
            return fd_.load(std::memory_order_acquire);
        }

        /**
         * @brief Check if file descriptor is valid
         * @return true if file descriptor is valid (>= 0)
         */
        [[nodiscard]] bool isValid() const
        {
            return fd_.load(std::memory_order_acquire) >= 0;
        }

        /**
         * @brief Close the file descriptor
         */
        void close()
        {
            const int fd = fd_.exchange(-1, std::memory_order_acq_rel);
            if (fd >= 0)
            {
                ::close(fd);
            }
        }

        /**
         * @brief Reset with new file descriptor (closes old one)
         * @param fd New file descriptor value
         */
        void reset(const int fd = -1)
        {
            close();
            fd_.store(fd, std::memory_order_release);
        }

        /**
         * @brief Release ownership of file descriptor
         * @return The file descriptor value (caller is responsible for closing)
         */
        int release()
        {
            return fd_.exchange(-1, std::memory_order_acq_rel);
        }

        /**
         * @brief Implicit conversion to int for backward compatibility
         */
        explicit operator int() const
        {
            return get();
        }

    private:
        std::atomic<int> fd_;
    };

    /**
     * @brief Scoped lock for file descriptor operations
     * 
     * Provides thread-safe access to file descriptor operations
     * by ensuring exclusive access during the scope.
     */
    class FdLock
    {
    public:
        /**
         * @brief Construct and lock the mutex
         * @param mutex The mutex to lock
         */
        explicit FdLock(std::mutex& mutex) : lock_(mutex) {}

        /// @brief Prevent copying
        FdLock(const FdLock&) = delete;
        FdLock& operator=(const FdLock&) = delete;

    private:
        std::lock_guard<std::mutex> lock_;
    };

} // namespace mex_hal

#endif // MEX_HAL_FILE_DESCRIPTOR_H
