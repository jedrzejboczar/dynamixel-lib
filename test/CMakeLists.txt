enable_testing()

add_executable(dynamixel-packet-tests ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel-packet-tests.c)

find_package(cmocka REQUIRED)
target_link_libraries(dynamixel-packet-tests PRIVATE dynamixel)
target_link_libraries(dynamixel-packet-tests PRIVATE cmocka)

add_test(dynamixel-packet-tests ${CMAKE_CURRENT_BINARY_DIR}/dynamixel-packet-tests)
