cmake_minimum_required(VERSION 3.12)

# Find the HAL library
find_library(HAL_LIBRARY hal PATHS ${CMAKE_INSTALL_PREFIX}/lib)
find_path(HAL_INCLUDE_DIR hal/core.h PATHS ${CMAKE_INSTALL_PREFIX}/include)

if(NOT HAL_LIBRARY OR NOT HAL_INCLUDE_DIR)
    message(STATUS "HAL library not found in system, using local build")
    set(HAL_LIBRARY hal)
    set(HAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
endif()

# GPIO Blink Example
add_executable(gpio_blink gpio_blink.cpp)
target_include_directories(gpio_blink PRIVATE ${HAL_INCLUDE_DIR})
target_link_libraries(gpio_blink PRIVATE ${HAL_LIBRARY} pthread)

# Timer Demo Example
add_executable(timer_demo timer_demo.cpp)
target_include_directories(timer_demo PRIVATE ${HAL_INCLUDE_DIR})
target_link_libraries(timer_demo PRIVATE ${HAL_LIBRARY} pthread)

# Installation
install(TARGETS gpio_blink timer_demo
    RUNTIME DESTINATION bin/examples
)