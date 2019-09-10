#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "packet.h"

/*
 * Functions that prepare instruction packets to be sent.
 * The packet can be then sent in the following way:
 *   void uart_send(uint8_t *data, int size);  <- assuming this declaration
 *   uart_send(dynamixel_packet_data(packet), dynamixel_packet_size(packet));
 * Each function returns expected number of data to be received
 *  or -1 on error (packet too long).
 * Then the received packet can be checked:
 *   bool is_ok = dynamixel_packet_checksum_isok(packet)
 */


// for simple instructions with no parameters: ping, action, reset
int dynamixel_prepare_simple_instruction(DynamixelPacket *packet, uint8_t id, uint8_t instruction);

int dynamixel_prepare_ping(DynamixelPacket *packet, uint8_t id);
int dynamixel_prepare_action(DynamixelPacket *packet, uint8_t id);
int dynamixel_prepare_reset(DynamixelPacket *packet, uint8_t id);
int dynamixel_prepare_write(DynamixelPacket *packet, uint8_t id, uint8_t address, const uint8_t *data, int data_len);
int dynamixel_prepare_read(DynamixelPacket *packet, uint8_t id, uint8_t address, int data_len);
int dynamixel_prepare_reg_write(DynamixelPacket *packet, uint8_t id, uint8_t address, const uint8_t *data, int data_len);
// Allows to send data to many dynamixel serovs in one packet, always uses broadcasting ID
// expects data to be an array in format:
// {id1, data11, data12, ..., data1L, id2, data21, ..., data2L, ..., idN, dataN1, ..., dataNL},
// where: N - number of dynamixel actuator (servo) "subpackets",
//        L - length of data for each "subpacket" (excluding ID; all lengths are the same)
// the total size of data array can be computed as: (L + 1) * N
int dynamixel_prepare_sync_write(DynamixelPacket *packet, uint8_t address, const uint8_t *data,
        int n_actuators, int data_len_for_each);

// Different interface for constructins sync-write packets (probably more convenient)
// First initialize the packet with _init, specifying control table address to be written
// and length of data for each servo.
// Then call _add_next for each servo; actuator_data should have length equalt to data_len_for_each.
// Last call _end to finalize the packet
int dynamixel_prepare_sync_write_init(DynamixelPacket *packet, uint8_t address, int data_len_for_each);
int dynamixel_prepare_sync_write_add_next(DynamixelPacket *packet, uint8_t id, const uint8_t *actuator_data);
int dynamixel_prepare_sync_write_end(DynamixelPacket *packet);

// convenient wrappers around simple reading and writing of a single register
int dynamixel_prepare_set_register_u8(DynamixelPacket *packet, uint8_t id, uint8_t address, uint8_t value);
int dynamixel_prepare_set_register_u16(DynamixelPacket *packet, uint8_t id, uint8_t address, uint16_t value);
int dynamixel_prepare_read_register_u8(DynamixelPacket *packet, uint8_t id, uint8_t address);
int dynamixel_prepare_read_register_u16(DynamixelPacket *packet, uint8_t id, uint8_t address);


/*
 * Angle conversions
 * Internally angles 0-300 deg are represented as 0-1023
 *
 */
float dynamixel_angle2deg(uint16_t angle_int);  // internal -> float degrees
float dynamixel_angle2rad(uint16_t angle_int);  // internal -> float radians
uint16_t dynamixel_deg2angle(float angle_deg);  // float degrees -> internal
uint16_t dynamixel_rad2angle(float angle_rad);  // float radians -> internal


#ifdef __cplusplus
}
#endif

