# a minimal CMake configuration
cmake_minimum_required(VERSION 3.8)
project(dynamixel)

option(WITH_FREERTOS "Include FreeRTOS part of the library (requires FreeRTOS)")
option(DISCOVERY_UTILS "Include utilities for easy discovery of servo numbers on the line")

add_library(dynamixel STATIC "")
add_subdirectory(src)

add_subdirectory(test)
