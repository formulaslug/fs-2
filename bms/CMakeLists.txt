# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET bms)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

# Libraries
add_subdirectory(${MBED_PATH})
add_subdirectory(lib-mbed-ltc681x)

# Remove warnings that cause gcc to fail
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register -Wno-deprecated-declarations")

add_executable(${APP_TARGET})

mbed_configure_app_target(${APP_TARGET})

target_sources(${APP_TARGET}
    PRIVATE
        src/Main.cpp
        src/BmsThread.cpp
        src/EnergusTempSensor.cpp
        src/LTC6811.cpp
        src/Can.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-events
        lib-mbed-ltc681x
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
