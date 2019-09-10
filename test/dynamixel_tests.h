#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "dynamixel.h"

static void test_prepare_ping(void **state) {
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_ping(&packet, 1);
    uint8_t correct_packet[] = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, 6);
}

static void test_prepare_write(void **state) {
    // Example 8
    {
        uint8_t correct_packet[] = {0xff, 0xff, 0x00, 0x04, 0x03, 0x04, 0x01, 0xf3};
        uint8_t correct_response_size = 6;
        DynamixelPacket packet;
        uint8_t data[] = {0x01};
        int response_size = dynamixel_prepare_write(&packet, 0, 0x04, data, 1);
        assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
        assert_int_equal(response_size, correct_response_size);
    }
    // Example 10
    {
        uint8_t correct_packet[] = {0xff, 0xff, 0x00, 0x05, 0x03, 0x08, 0xff, 0x01, 0xef};
        uint8_t correct_response_size = 6;
        DynamixelPacket packet;
        uint8_t data[] = {0xff, 0x01};
        int response_size = dynamixel_prepare_write(&packet, 0, 0x08, data, 2);
        assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
        assert_int_equal(response_size, correct_response_size);
    }
}

static void test_prepare_write_broadcasting(void **state) {
    // Example 1
    uint8_t correct_packet[] = {0xff, 0xff, 0xfe, 0x04, 0x03, 0x03, 0x01, 0xf6};
    uint8_t correct_response_size = 0; // broadcasting
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_set_register_u8(&packet, 0xfe, 0x03, 1);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_read(void **state) {
    // Example 6
    uint8_t correct_packet[] = {0xff, 0xff, 0x01, 0x04, 0x02, 0x00, 0x03, 0xf5};
    uint8_t correct_response_size = 9;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_read(&packet, 1, 0x00, 3);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_reg_write(void **state) {
    // Example 19
    uint8_t correct_packet1[] = {0xff, 0xff, 0x00, 0x05, 0x04, 0x1e, 0x00, 0x00, 0xd8};
    uint8_t correct_packet2[] = {0xff, 0xff, 0x01, 0x05, 0x04, 0x1e, 0xff, 0x03, 0xd5};
    uint8_t data1[] = {0x00, 0x00};
    uint8_t data2[] = {0xff, 0x03};
    uint8_t correct_response_size = 6;
    DynamixelPacket packet;
    int response_size;
    response_size = dynamixel_prepare_reg_write(&packet, 0, 0x1e, data1, 2);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet1, sizeof(correct_packet1));
    assert_int_equal(response_size, correct_response_size);
    response_size = dynamixel_prepare_reg_write(&packet, 1, 0x1e, data2, 2);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet2, sizeof(correct_packet2));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_action(void **state) {
    // Example 19
    uint8_t correct_packet[] = {0xff, 0xff, 0xfe, 0x02, 0x05, 0xfa};
    uint8_t correct_response_size = 0;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_action(&packet, 0xfe);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_reset(void **state) {
    // Example 4
    uint8_t correct_packet[] = {0xff, 0xff, 0x00, 0x02, 0x06, 0xf7};
    uint8_t correct_response_size = 6;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_reset(&packet, 0x00);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_sync_write(void **state) {
    // Example 5
    uint8_t correct_packet[] = {0xff, 0xff, 0xfe, 0x18, 0x83,
        0x1e, 0x04, // starting address and L
        0x00, 0x10, 0x00, 0x50, 0x01,
        0x01, 0x20, 0x02, 0x60, 0x03,
        0x02, 0x30, 0x00, 0x70, 0x01,
        0x03, 0x20, 0x02, 0x80, 0x03, // Error in documentation (written ID=0 but data for ID=3)
        0x12};
    uint8_t correct_response_size = 0; // always broadcasting
    DynamixelPacket packet;
    uint8_t data[] = {
        0x00, 0x10, 0x00, 0x50, 0x01, // uint16 bytes have to be swapped (little endian)
        0x01, 0x20, 0x02, 0x60, 0x03,
        0x02, 0x30, 0x00, 0x70, 0x01,
        0x03, 0x20, 0x02, 0x80, 0x03,
    };
    int response_size = dynamixel_prepare_sync_write(&packet, 0x1e, data, 4, 4);
    assert_int_not_equal(response_size, -1); // maximum number of parameters too small!
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_sync_write_incremental(void **state) {
    // Example 5
    uint8_t correct_packet[] = {0xff, 0xff, 0xfe, 0x18, 0x83,
        0x1e, 0x04, // starting address and L
        0x00, 0x10, 0x00, 0x50, 0x01,
        0x01, 0x20, 0x02, 0x60, 0x03,
        0x02, 0x30, 0x00, 0x70, 0x01,
        0x03, 0x20, 0x02, 0x80, 0x03, // Error in documentation (written ID=0 but data for ID=3)
        0x12};
    uint8_t correct_response_size = 0; // always broadcasting
    DynamixelPacket packet;
    uint8_t id[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t data[][4] = {
        {0x10, 0x00, 0x50, 0x01}, // uint16 bytes have to be swapped (little endian)
        {0x20, 0x02, 0x60, 0x03},
        {0x30, 0x00, 0x70, 0x01},
        {0x20, 0x02, 0x80, 0x03},
    };
    int response_size = dynamixel_prepare_sync_write_init(&packet, 0x1e, 4);
    assert_int_equal(response_size, correct_response_size);
    // change length, as it is not yet fully assembled
    uint8_t final_len = correct_packet[3];
    correct_packet[3] = 2 + 2;
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, 7);
    for (int i = 0; i < 4; i++) {
        response_size = dynamixel_prepare_sync_write_add_next(&packet, id[i], data[i]);
        assert_int_equal(response_size, correct_response_size);
        correct_packet[3] = 2 + 2 + (4+1) * (i+1);
        assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, 7 + 4*(i+1));
    }
    response_size = dynamixel_prepare_sync_write_end(&packet);
    assert_int_equal(response_size, correct_response_size);
    correct_packet[3] = final_len;
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
}

static void test_prepare_set_register_u8(void **state) {
    // Example 7
    uint8_t correct_packet[] = {0xff, 0xff, 0x01, 0x04, 0x03, 0x03, 0x00, 0xf4};
    uint8_t correct_response_size = 6;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_set_register_u8(&packet, 1, 0x03, 0);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_set_register_u16(void **state) {
    // Example 10
    uint8_t correct_packet[] = {0xff, 0xff, 0x00, 0x05, 0x03, 0x08, 0xff, 0x01, 0xef};
    uint8_t correct_response_size = 6;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_set_register_u16(&packet, 0, 0x08, 0x01ff);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_read_register_u8(void **state) {
    // Example 2
    uint8_t correct_packet[] = {0xff, 0xff, 0x01, 0x04, 0x02, 0x2b, 0x01, 0xcc};
    uint8_t correct_response_size = 7;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_read_register_u8(&packet, 1, 0x2b);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

static void test_prepare_read_register_u16(void **state) {
    // modified Example 2
    uint8_t correct_packet[] = {0xff, 0xff, 0x01, 0x04, 0x02, 0x2b, 0x02,
        (uint8_t) ~(0x01 + 0x04 + 0x02 + 0x2b + 0x02)};
    uint8_t correct_response_size = 8;
    DynamixelPacket packet;
    int response_size = dynamixel_prepare_read_register_u16(&packet, 1, 0x2b);
    assert_memory_equal(dynamixel_packet_data(&packet), correct_packet, sizeof(correct_packet));
    assert_int_equal(response_size, correct_response_size);
}

int run_dynamixel_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_prepare_ping),
        cmocka_unit_test(test_prepare_write),
        cmocka_unit_test(test_prepare_write_broadcasting),
        cmocka_unit_test(test_prepare_read),
        cmocka_unit_test(test_prepare_reg_write),
        cmocka_unit_test(test_prepare_action),
        cmocka_unit_test(test_prepare_reset),
        cmocka_unit_test(test_prepare_sync_write),
        cmocka_unit_test(test_prepare_sync_write_incremental),
        cmocka_unit_test(test_prepare_set_register_u8),
        cmocka_unit_test(test_prepare_set_register_u16),
        cmocka_unit_test(test_prepare_read_register_u8),
        cmocka_unit_test(test_prepare_read_register_u16),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

