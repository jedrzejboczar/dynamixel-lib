#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "packet.h"


// static int setup(void **state) {
//     uint8_t ping_packet = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
//     uint8_t ping_response = {0xff 0xff 0x01 0x02 0x00 0xfc};
//
//     DynamixelPacket write_packet;
//     dynamixel_packet_init(&packet);
//
//         int *answer = malloc(sizeof(int));
//     if (*answer == NULL) {
//         return -1;
//     }
//     *answer = 42;
//     *state = answer;
//     return 0;
// }
// static int teardown(void **state) {
//     free(*state);
//     return 0;
// }

static void test_DynamixelPacket_struct_size(void **state) {
    assert_int_equal(sizeof(DynamixelPacket), 2 + 3 + DYNAMIXEL_MAX_N_PARAMETERS + 1);
}

static void test_dynamixel_packet_init(void **state) {
    DynamixelPacket packet;
    dynamixel_packet_init(&packet, 1, 1);
    uint8_t correct_memory[sizeof(DynamixelPacket)] = {0};
    correct_memory[0] = 0xff;
    correct_memory[1] = 0xff;
    correct_memory[2] = 0x01;
    correct_memory[3] = 0x02;
    correct_memory[4] = 0x01;
    assert_memory_equal(dynamixel_packet_data(&packet), correct_memory, sizeof(correct_memory));
}

static void test_dynamixel_packet_get_checksum_index(void **state) {
    DynamixelPacket packet = {0};
    packet.length = 2;
    assert_int_equal(dynamixel_packet_get_checksum_index(&packet), 0);
    packet.length = 5;
    assert_int_equal(dynamixel_packet_n_parameters(&packet), 3);
}

static void test_dynamixel_packet_get_checksum(void **state) {
    uint8_t packet[] = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
    assert_int_equal(dynamixel_packet_get_checksum((DynamixelPacket *) packet), 0xfb);
}

static void test_dynamixel_packet_compute_checksum(void **state) {
    uint8_t packet[] = {0xff, 0xff, 0x01, 0x02, 0x01}; // without checksum
    assert_int_equal(dynamixel_packet_compute_checksum((DynamixelPacket *) packet), 0xfb);
    assert_int_equal((uint8_t) ~(0x01 + 0x02 + 0x01), 0xfb);
}

static void test_dynamixel_packet_checksum_isok(void **state) {
    uint8_t packet_good[] = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
    uint8_t packet_bad[]  = {0xff, 0xff, 0x01, 0x02, 0x01, 0xeb};
    assert_true(dynamixel_packet_checksum_isok((DynamixelPacket *) packet_good));
    assert_false(dynamixel_packet_checksum_isok((DynamixelPacket *) packet_bad));
}

static void test_dynamixel_packet_n_parameters(void **state) {
    DynamixelPacket packet = {0};
    packet.length = 2;
    assert_int_equal(dynamixel_packet_n_parameters(&packet), 0);
    packet.length = 5;
    assert_int_equal(dynamixel_packet_n_parameters(&packet), 3);
}

static void test_dynamixel_packet_add_parameter(void **state) {
    DynamixelPacket packet;
    dynamixel_packet_init(&packet, 1, 0x01);
    assert_int_equal(packet.length, 2);
    assert_int_equal(packet.parameters_with_checksum[0], 0);
    assert_int_equal(packet.parameters_with_checksum[1], 0);
    assert_int_equal(packet.parameters_with_checksum[2], 0);
    dynamixel_packet_add_parameter(&packet, 0xee);
    assert_int_equal(packet.length, 3);
    assert_int_equal(packet.parameters_with_checksum[0], 0xee);
    assert_int_equal(packet.parameters_with_checksum[1], 0);
    assert_int_equal(packet.parameters_with_checksum[2], 0);
    dynamixel_packet_add_parameter(&packet, 0xab);
    assert_int_equal(packet.length, 4);
    assert_int_equal(packet.parameters_with_checksum[0], 0xee);
    assert_int_equal(packet.parameters_with_checksum[1], 0xab);
    assert_int_equal(packet.parameters_with_checksum[2], 0);
}

static void test_dynamixel_packet_add_parameter_u16(void **state) {
    DynamixelPacket packet;
    dynamixel_packet_init(&packet, 1, 0x01);
    assert_int_equal(packet.length, 2);
    assert_int_equal(packet.parameters_with_checksum[0], 0);
    assert_int_equal(packet.parameters_with_checksum[1], 0);
    assert_int_equal(packet.parameters_with_checksum[2], 0);
    dynamixel_packet_add_parameter_u16(&packet, 0x01ff); // like in Example 10
    assert_int_equal(packet.length, 4);
    assert_int_equal(packet.parameters_with_checksum[0], 0xff);
    assert_int_equal(packet.parameters_with_checksum[1], 0x01);
    assert_int_equal(packet.parameters_with_checksum[2], 0);
}

static void test_dynamixel_packet_add_parameters(void **state) {
    DynamixelPacket packet;
    dynamixel_packet_init(&packet, 1, 0x01);
    uint8_t mem1[5] = {0};
    assert_int_equal(packet.length, 2);
    assert_memory_equal(packet.parameters_with_checksum, mem1, sizeof(mem1));
    uint8_t params[] = {0x1f, 0x54, 0xfa, 0x34, 0x60};
    dynamixel_packet_add_parameters(&packet, params, sizeof(params));
    assert_int_equal(packet.length, 2 + 5);
    assert_memory_equal(packet.parameters_with_checksum, params, sizeof(params));
}

static void test_dynamixel_packet_data(void **state) {
    DynamixelPacket packet;
    assert_ptr_equal(&packet, dynamixel_packet_data(&packet));
}

static void test_dynamixel_packet_size(void **state) {
    uint8_t packet[] = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
    assert_int_equal(dynamixel_packet_size((DynamixelPacket *) packet), sizeof(packet));
}

static void test_dynamixel_packet_space_remaining(void **state) {
    DynamixelPacket p;
    uint8_t packet1[] = {0xff, 0xff, 0x01, 0x02, 0x01, 0xfb};
    uint8_t packet2[] = {0xff, 0xff, 0xfe, 0x04, 0x03, 0x03, 0x01, 0xf6};
    assert_int_equal(dynamixel_packet_space_remaining((DynamixelPacket *) packet1),
            sizeof(p.parameters_with_checksum));
    assert_int_equal(dynamixel_packet_space_remaining((DynamixelPacket *) packet2),
            sizeof(p.parameters_with_checksum) - 2);
}


int run_dynamixel_packet_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_DynamixelPacket_struct_size),
        cmocka_unit_test(test_dynamixel_packet_init),
        cmocka_unit_test(test_dynamixel_packet_get_checksum_index),
        cmocka_unit_test(test_dynamixel_packet_get_checksum),
        cmocka_unit_test(test_dynamixel_packet_compute_checksum),
        cmocka_unit_test(test_dynamixel_packet_checksum_isok),
        cmocka_unit_test(test_dynamixel_packet_n_parameters),
        cmocka_unit_test(test_dynamixel_packet_add_parameter),
        cmocka_unit_test(test_dynamixel_packet_add_parameter_u16),
        cmocka_unit_test(test_dynamixel_packet_add_parameters),
        cmocka_unit_test(test_dynamixel_packet_data),
        cmocka_unit_test(test_dynamixel_packet_size),
        cmocka_unit_test(test_dynamixel_packet_space_remaining),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}



