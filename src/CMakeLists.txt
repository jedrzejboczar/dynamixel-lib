target_include_directories(dynamixel PUBLIC .)

target_sources(dynamixel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/packet.c
    )

if(WITH_FREERTOS)
    find_package(FreeRTOS REQUIRED)
    target_sources(dynamixel PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/io_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/servo_group.cpp
        )
    target_link_libraries(dynamixel PUBLIC FreeRTOS)
endif()

# used rather for debugging purposes
if(DISCOVERY_UTILS)
    target_sources(dynamixel
        ${CMAKE_CURRENT_SOURCE_DIR}/discovery_utils.c
        )
endif()

