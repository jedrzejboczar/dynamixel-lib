#include <string.h> // memset, memcpy

#include "packet.h"


void dynamixel_packet_init(DynamixelPacket *packet, uint8_t id, uint8_t instruction) {
    // dynamixel start bytes are always 0xff 0xff
    packet->_start_bytes[0] = 0xff;
    packet->_start_bytes[1] = 0xff;
    packet->id = id;
    packet->length = 2; // instruction/error and checksum
    packet->instruction = instruction;
    memset(packet->parameters_with_checksum, 0,
            sizeof(packet->parameters_with_checksum) / sizeof(packet->parameters_with_checksum[0]));
}

bool dynamixel_packet_add_parameter(DynamixelPacket *packet, uint8_t parameter) {
    int next_index = dynamixel_packet_n_parameters(packet);
    int array_max_size = sizeof(packet->parameters_with_checksum) / sizeof(packet->parameters_with_checksum[0]);
    if (next_index >= array_max_size)
        return false; // cannot add more elements
    packet->parameters_with_checksum[next_index] = parameter;
    packet->length++;
    return true;
}

bool dynamixel_packet_add_parameters(DynamixelPacket *packet, const uint8_t *parameters, int n_parameters) {
    int next_index = dynamixel_packet_n_parameters(packet);
    int space_remaining = dynamixel_packet_space_remaining(packet);
    if (n_parameters > space_remaining)
        return false; // cannot add all of the elements, so do not add anything
    memcpy(&packet->parameters_with_checksum[next_index], parameters, n_parameters);
    packet->length += n_parameters;
    return true;
}

bool dynamixel_packet_add_parameter_u16(DynamixelPacket *packet, uint16_t parameter) {
    // FIXME: endianess-dependent
    return dynamixel_packet_add_parameters(packet, (uint8_t *) &parameter, 2);
}

bool dynamixel_packet_add_checksum(DynamixelPacket *packet) {
    int space_remaining = dynamixel_packet_space_remaining(packet);
    if (space_remaining < 1)
        return false;
    int checksum_index = dynamixel_packet_get_checksum_index(packet);
    uint8_t checksum = dynamixel_packet_compute_checksum(packet);
    packet->parameters_with_checksum[checksum_index] = checksum;
    return true;
}

/*  */

uint8_t* dynamixel_packet_data(DynamixelPacket *packet) {
    // DynamixelPacket is a packed structure, so we can send it directly
    return (uint8_t *) packet;
}

int dynamixel_packet_size(DynamixelPacket *packet) {

    return DYNAMIXEL_PACKET_BASE_SIZE + dynamixel_packet_n_parameters(packet) + 1;
}

int dynamixel_packet_space_remaining(DynamixelPacket *packet) {
    int next_index = dynamixel_packet_n_parameters(packet);
    int array_max_size = sizeof(packet->parameters_with_checksum) / sizeof(packet->parameters_with_checksum[0]);
    int space_remaining = array_max_size - next_index;
    return space_remaining;
}

int dynamixel_packet_n_parameters(DynamixelPacket *packet) {
    return packet->length - 2;
}

int dynamixel_packet_get_checksum_index(DynamixelPacket *packet) {
    // it is the element after last parameter, so we can index array[n_parameters]
    return dynamixel_packet_n_parameters(packet);
}

uint8_t dynamixel_packet_get_checksum(DynamixelPacket *packet) {
    int checksum_index = dynamixel_packet_get_checksum_index(packet);
    return packet->parameters_with_checksum[checksum_index];
}

uint8_t dynamixel_packet_compute_checksum(DynamixelPacket *packet) {
    // checksum is the bitwise NOT of sum of:
    // id, length, instruction/error, all parameters
    uint32_t checksum = 0;
    checksum += packet->id;
    checksum += packet->length;
    checksum += packet->instruction;
    for (int i = 0; i < dynamixel_packet_n_parameters(packet); i++)
        checksum += packet->parameters_with_checksum[i];
    checksum = ~checksum;
    return 0xff & checksum;
}

bool dynamixel_packet_checksum_isok(DynamixelPacket *packet) {
    uint8_t current_checksum = dynamixel_packet_get_checksum(packet);
    return dynamixel_packet_compute_checksum(packet) == current_checksum;
}

/*** Private functions *********************************************************/
