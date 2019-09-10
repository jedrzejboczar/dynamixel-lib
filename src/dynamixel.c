#include <assert.h>
#include "dynamixel.h"

#define CHECKSUM 1   // be more verbose


int dynamixel_prepare_write(DynamixelPacket *packet, uint8_t id, uint8_t address, const uint8_t *data, int data_len) {
    dynamixel_packet_init(packet, id, DYNAMIXEL_INST_WRITE);
    bool is_ok = dynamixel_packet_add_parameter(packet, address);
    is_ok = is_ok && dynamixel_packet_add_parameters(packet, data, data_len);
    is_ok = is_ok && dynamixel_packet_add_checksum(packet);
    if (!is_ok)
        return -1;
    if (id == DYNAMIXEL_BROADCASTING_ID)
        return 0;
    return DYNAMIXEL_PACKET_BASE_SIZE + CHECKSUM;
}


int dynamixel_prepare_read(DynamixelPacket *packet, uint8_t id, uint8_t address, int data_len) {
    dynamixel_packet_init(packet, id, DYNAMIXEL_INST_READ);
    uint8_t data[] = {address, data_len};
    bool is_ok = dynamixel_packet_add_parameters(packet, data, 2);
    is_ok = is_ok && dynamixel_packet_add_checksum(packet);
    if (!is_ok)
        return -1;
    if (id == DYNAMIXEL_BROADCASTING_ID)
        return 0;
    return DYNAMIXEL_PACKET_BASE_SIZE + data_len + CHECKSUM;
}

int dynamixel_prepare_reg_write(DynamixelPacket *packet, uint8_t id, uint8_t address, const uint8_t *data, int data_len) {
    dynamixel_packet_init(packet, id, DYNAMIXEL_INST_REG_WRITE);
    bool is_ok = dynamixel_packet_add_parameter(packet, address);
    is_ok = is_ok && dynamixel_packet_add_parameters(packet, data, data_len);
    is_ok = is_ok && dynamixel_packet_add_checksum(packet);
    if (!is_ok)
        return -1;
    if (id == DYNAMIXEL_BROADCASTING_ID)
        return 0;
    return DYNAMIXEL_PACKET_BASE_SIZE + CHECKSUM;
}

int dynamixel_prepare_sync_write(DynamixelPacket *packet, uint8_t address, const uint8_t *data, int n_actuators, int data_len_for_each) {
    dynamixel_packet_init(packet, DYNAMIXEL_BROADCASTING_ID, DYNAMIXEL_INST_SYNC_WRITE);
    bool is_ok = dynamixel_packet_add_parameter(packet, address);
    is_ok = is_ok && dynamixel_packet_add_parameter(packet, data_len_for_each);
    is_ok = is_ok && dynamixel_packet_add_parameters(packet, data, (data_len_for_each + 1) * n_actuators);
    is_ok = is_ok && dynamixel_packet_add_checksum(packet);
    if (!is_ok)
        return -1;
    return 0; // always broadcasting
}

int dynamixel_prepare_sync_write_init(DynamixelPacket *packet, uint8_t address, int data_len_for_each) {
    dynamixel_packet_init(packet, DYNAMIXEL_BROADCASTING_ID, DYNAMIXEL_INST_SYNC_WRITE);
    bool is_ok = dynamixel_packet_add_parameter(packet, address);
    is_ok = is_ok && dynamixel_packet_add_parameter(packet, data_len_for_each);
    if (!is_ok)
        return -1;
    return 0;
}

int dynamixel_prepare_sync_write_add_next(DynamixelPacket *packet, uint8_t id, const uint8_t *actuator_data) {
    int data_len_for_each = packet->parameters_with_checksum[1]; // it is the second parameter
    bool is_ok = dynamixel_packet_add_parameter(packet, id);
    is_ok = is_ok && dynamixel_packet_add_parameters(packet, actuator_data, data_len_for_each);
    if (!is_ok)
        return -1;
    return 0;
}

int dynamixel_prepare_sync_write_end(DynamixelPacket *packet) {
    bool is_ok = dynamixel_packet_add_checksum(packet);
    if (!is_ok)
        return -1;
    return 0;
}


int dynamixel_prepare_simple_instruction(DynamixelPacket *packet, uint8_t id, uint8_t instruction) {
    dynamixel_packet_init(packet, id, instruction);
    dynamixel_packet_add_checksum(packet);
    assert(packet->length == 0x02);
    if (id == DYNAMIXEL_BROADCASTING_ID)
        return 0;  // no responese when breadcasting
    // returns no parameters, just checksum
    return DYNAMIXEL_PACKET_BASE_SIZE + CHECKSUM;
}

int dynamixel_prepare_ping(DynamixelPacket *packet, uint8_t id) {
    return dynamixel_prepare_simple_instruction(packet, id, DYNAMIXEL_INST_PING);
}

int dynamixel_prepare_action(DynamixelPacket *packet, uint8_t id) {
    return dynamixel_prepare_simple_instruction(packet, id, DYNAMIXEL_INST_ACTION);
}

int dynamixel_prepare_reset(DynamixelPacket *packet, uint8_t id) {
    return dynamixel_prepare_simple_instruction(packet, id, DYNAMIXEL_INST_RESET);
}

int dynamixel_prepare_set_register_u8(DynamixelPacket *packet, uint8_t id, uint8_t address, uint8_t value) {
    return dynamixel_prepare_write(packet, id, address, &value, 1);
}

int dynamixel_prepare_set_register_u16(DynamixelPacket *packet, uint8_t id, uint8_t address, uint16_t value) {
    // uint8_t data[] = {value & 0xff, value >> 8}; TODO: does casting work properly?
    return dynamixel_prepare_write(packet, id, address, (uint8_t *) &value, 2);
}

int dynamixel_prepare_read_register_u8(DynamixelPacket *packet, uint8_t id, uint8_t address) {
    return dynamixel_prepare_read(packet, id, address, 1);
}

int dynamixel_prepare_read_register_u16(DynamixelPacket *packet, uint8_t id, uint8_t address) {
    return dynamixel_prepare_read(packet, id, address, 2);
}




float dynamixel_angle2deg(uint16_t angle_int) {
    assert(angle_int <= DYNAMIXEL_MAX_ANGLE_INT);
    return (float) angle_int / DYNAMIXEL_MAX_ANGLE_INT * DYNAMIXEL_MAX_ANGLE_DEG;
}

float dynamixel_angle2rad(uint16_t angle_int) {
    assert(angle_int <= DYNAMIXEL_MAX_ANGLE_INT);
    return (float) angle_int / DYNAMIXEL_MAX_ANGLE_INT * DYNAMIXEL_MAX_ANGLE_RAD;
}

uint16_t dynamixel_deg2angle(float angle_deg) {
    assert(angle_deg <= DYNAMIXEL_MAX_ANGLE_DEG);
    return angle_deg / DYNAMIXEL_MAX_ANGLE_DEG * DYNAMIXEL_MAX_ANGLE_INT;
}

uint16_t dynamixel_rad2angle(float angle_rad) {
    assert(angle_rad <= DYNAMIXEL_MAX_ANGLE_RAD);
    return angle_rad / DYNAMIXEL_MAX_ANGLE_RAD * DYNAMIXEL_MAX_ANGLE_INT;
}


#undef CHECKSUM // was needed only here
