cmake_minimum_required(VERSION 3.12)

project(mex-hal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HAL_VERSION_MAJOR 1)
set(HAL_VERSION_MINOR 0)
set(HAL_VERSION_PATCH 0)

set(HAL_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/hal/version.h")

file(WRITE ${HAL_VERSION_HEADER}
        "#pragma once
         #define HAL_VERSION_MAJOR ${HAL_VERSION_MAJOR}
         #define HAL_VERSION_MINOR ${HAL_VERSION_MINOR}
         #define HAL_VERSION_PATCH ${HAL_VERSION_PATCH}
         #define HAL_VERSION \"${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH}\"
        ")

message(STATUS "Building mex-hal version ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH}")

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_RT "Build with realtime support" ON)
option(BUILD_SIMULATOR "Build simulator backends" OFF)

find_package(Threads REQUIRED)

# Realtime requirements
if(BUILD_RT)
    find_library(RT_LIBRARY rt)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
endif()

add_library(hal-core
        src/core.cpp
        src/resource_manager.cpp
        src/callback_manager.cpp
)

# Interface libraries for each module
# GPIO
add_library(hal-gpio-interface INTERFACE)
target_sources(hal-gpio-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/gpio.h)
target_link_libraries(hal-gpio-interface INTERFACE hal-core)

add_library(hal-gpio-linux src/gpio/gpio_linux.cpp
        src/device_config/device_config.h)
target_link_libraries(hal-gpio-linux PRIVATE hal-gpio-interface)

if(BUILD_SIMULATOR)
    add_library(hal-gpio-simulator src/gpio/gpio_simulator.cpp)
    target_link_libraries(hal-gpio-simulator PRIVATE hal-gpio-interface)
endif()

# SPI
add_library(hal-spi-interface INTERFACE)
target_sources(hal-spi-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/spi.h)
target_link_libraries(hal-spi-interface INTERFACE hal-core)

add_library(hal-spi-linux src/spi/spi_linux.cpp)
target_link_libraries(hal-spi-linux PRIVATE hal-spi-interface)

if(BUILD_SIMULATOR)
    add_library(hal-spi-simulator src/spi/spi_simulator.cpp)
    target_link_libraries(hal-spi-simulator PRIVATE hal-spi-interface)
endif()

# I2C
add_library(hal-i2c-interface INTERFACE)
target_sources(hal-i2c-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/i2c.h)
target_link_libraries(hal-i2c-interface INTERFACE hal-core)

add_library(hal-i2c-linux src/i2c/i2c_linux.cpp)
target_link_libraries(hal-i2c-linux PRIVATE hal-i2c-interface)

if(BUILD_SIMULATOR)
    add_library(hal-i2c-simulator src/i2c/i2c_simulator.cpp)
    target_link_libraries(hal-i2c-simulator PRIVATE hal-i2c-interface)
endif()

# PWM
add_library(hal-pwm-interface INTERFACE)
target_sources(hal-pwm-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/pwm.h)
target_link_libraries(hal-pwm-interface INTERFACE hal-core)

add_library(hal-pwm-linux src/pwm/pwm_linux.cpp)
target_link_libraries(hal-pwm-linux PRIVATE hal-pwm-interface)

if(BUILD_SIMULATOR)
    add_library(hal-pwm-simulator src/pwm/pwm_simulator.cpp)
    target_link_libraries(hal-pwm-simulator PRIVATE hal-pwm-interface)
endif()

# UART
add_library(hal-uart-interface INTERFACE)
target_sources(hal-uart-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/uart.h)
target_link_libraries(hal-uart-interface INTERFACE hal-core)

add_library(hal-uart-linux src/uart/uart_linux.cpp)
target_link_libraries(hal-uart-linux PRIVATE hal-uart-interface)

if(BUILD_SIMULATOR)
    add_library(hal-uart-simulator src/uart/uart_simulator.cpp)
    target_link_libraries(hal-uart-simulator PRIVATE hal-uart-interface)
endif()

# ADC
add_library(hal-adc-interface INTERFACE)
target_sources(hal-adc-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/adc.h)
target_link_libraries(hal-adc-interface INTERFACE hal-core)

add_library(hal-adc-linux src/adc/adc_linux.cpp)
target_link_libraries(hal-adc-linux PRIVATE hal-adc-interface)

if(BUILD_SIMULATOR)
    add_library(hal-adc-simulator src/adc/adc_simulator.cpp)
    target_link_libraries(hal-adc-simulator PRIVATE hal-adc-interface)
endif()

# Timer
add_library(hal-timer-interface INTERFACE)
target_sources(hal-timer-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/hal/timer.h)
target_link_libraries(hal-timer-interface INTERFACE hal-core)

add_library(hal-timer-linux src/timer/timer_linux.cpp)
target_link_libraries(hal-timer-linux PRIVATE hal-timer-interface)

if(BUILD_SIMULATOR)
    add_library(hal-timer-simulator src/timer/timer_simulator.cpp)
    target_link_libraries(hal-timer-simulator PRIVATE hal-timer-interface)
endif()

# Main HAL library
add_library(hal
        $<TARGET_OBJECTS:hal-core>
        $<TARGET_OBJECTS:hal-gpio-linux>
        $<TARGET_OBJECTS:hal-spi-linux>
        $<TARGET_OBJECTS:hal-i2c-linux>
        $<TARGET_OBJECTS:hal-pwm-linux>
        $<TARGET_OBJECTS:hal-uart-linux>
        $<TARGET_OBJECTS:hal-adc-linux>
        $<TARGET_OBJECTS:hal-timer-linux>
        src/hal_state_engine.cpp
        src/device_config/device_config.cpp
        src/sys_config/sys_config.cpp
        src/resource_visualizer.cpp
        src/resource_manager.cpp
        src/callback_manager.cpp
)
target_include_directories(hal PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(hal_main src/main.cpp)
target_link_libraries(hal_main PRIVATE hal)

if(BUILD_RT)
    target_link_libraries(hal PRIVATE ${RT_LIBRARY} Threads::Threads ${LIBXML2_LIBRARIES})
    target_include_directories(hal PRIVATE ${LIBXML2_INCLUDE_DIRS})
    target_compile_definitions(hal PRIVATE HAL_RT_ENABLED)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Building tests")
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "Building examples")
else()
    message(STATUS "Skipping examples build. Set BUILD_EXAMPLES to ON to include them.")
endif()

# Installation
install(TARGETS hal EXPORT hal-config DESTINATION lib)
install(DIRECTORY include/hal DESTINATION include)
install(EXPORT hal-config DESTINATION share/hal/cmake)