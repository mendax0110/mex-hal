cmake_minimum_required(VERSION 3.12)

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()
include(GoogleTest)

# Helper function to create test executables
function(add_hal_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE 
        hal
        gtest_main
        gmock_main
        Threads::Threads
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )
    gtest_discover_tests(${TEST_NAME})
endfunction()

# Core tests
add_hal_test(test_core test_core.cpp)
add_hal_test(test_resource_manager test_resource_manager.cpp)
add_hal_test(test_callback_manager test_callback_manager.cpp)

# Peripheral tests
add_hal_test(test_gpio test_gpio.cpp)
add_hal_test(test_spi test_spi.cpp)
add_hal_test(test_i2c test_i2c.cpp)
add_hal_test(test_uart test_uart.cpp)
add_hal_test(test_pwm test_pwm.cpp)
add_hal_test(test_timer test_timer.cpp)
add_hal_test(test_adc test_adc.cpp)

# Real-time tests
add_hal_test(test_realtime test_realtime.cpp)
