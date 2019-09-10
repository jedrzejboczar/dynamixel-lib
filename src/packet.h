#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Low level definition of a dynamixel packet and its methods
 * (would be a class in C++)
 *
 */

// this library currently casts uint16 directly to data being sent,
// thus it works only for little-endian targets (as Dynamixels are little-endian)
// TODO: fix all endainess dependet code?
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#   error "Some functions in this implementation are valid only on little-endian targets"
#endif


#include <stdbool.h>
#include <stdint.h>
// #include <stddef.h>

// As we don't want dynamic allocation we have to define a limit of parameters.
// This value is needed to statically allocate communication buffers.
// The size of the structure is in fact this+6 (but ofc use sizeof(DynamixelPacket))
#define DYNAMIXEL_MAX_N_PARAMETERS   24


/*
 * Structure that represents a dynamixel data packet.
 * Packets that are transmitted are Instruction Packets,
 * and those that are received are Status Packets.
 *
 * WARNING: Number of parameters may change, but there is always one
 *   byte of checksum at the end of parameters_with_checksum[] array!
 */
typedef struct __attribute__ ((__packed__)) {
    uint8_t _start_bytes[2];  // start bytes of dynamixel protocol (always the same)
    uint8_t id;               // id of dynamixel servo or BROADCASTING_ID
    uint8_t length;           // length of packet (= n_parameters + 2)
    union {
        uint8_t instruction;  // (transmitting) instruction code
        uint8_t error;        // (receiving) error status
    };
    // number of parameters with checksum at the end
    uint8_t parameters_with_checksum[DYNAMIXEL_MAX_N_PARAMETERS + 1];
} DynamixelPacket;


// size of the constant part of each packet which is always: start_bytes + id + length + instr/error
// it is the sisze of DynamixelPacket without parameters_with_checksum[] array
// useful for calculating size of reveice packets (as we don't have them yet created)
#define DYNAMIXEL_PACKET_BASE_SIZE   (2 + 3)


// initializes the structure with default values (other functions assume that this was called)
// needs to be called each time before adding parameters to a packet
void dynamixel_packet_init(DynamixelPacket *packet, uint8_t id, uint8_t instruction);
// appends next parameter and increments packet length
// returns false if parameters limit has been reached and does NOT add ANYTHING
bool dynamixel_packet_add_parameter(DynamixelPacket *packet, uint8_t parameter);
// appends parameters from array and increments packet length
// returns false if parameters limit has been reached and does NOT add ANYTHING
bool dynamixel_packet_add_parameters(DynamixelPacket *packet, const uint8_t *parameters, int n_parameters);
// convenience function - adds 2 parameters from uint16_t variable
// useful for setting values for registers split into low (L) and high (H) parts
// returns false if parameters limit has been reached and does NOT add ANYTHING
bool dynamixel_packet_add_parameter_u16(DynamixelPacket *packet, uint16_t parameter);
// adds checksum to packet already filled with all parameters
// returns false if there is not enough space and does NOT add ANYTHING
bool dynamixel_packet_add_checksum(DynamixelPacket *packet);

// returns pointer to packet data
uint8_t* dynamixel_packet_data(DynamixelPacket *packet);
// gives size (in bytes) of data packet to be transimitted through UART
int dynamixel_packet_size(DynamixelPacket *packet);
// returns number of elements that can still be added to packet->parameters_with_checksum array
// ! includes space for checksum (so if it returns 1, then there is space for checksum only)
int dynamixel_packet_space_remaining(DynamixelPacket *packet);

// returns the current number of parameters in the packet (different than packet.length)
int dynamixel_packet_n_parameters(DynamixelPacket *packet);
// finds out correct position of the checksum
int dynamixel_packet_get_checksum_index(DynamixelPacket *packet);
// used to access checksum (it finds out correct position of the checksum)
uint8_t dynamixel_packet_get_checksum(DynamixelPacket *packet);
// returns the checksum based on the data in a packet
uint8_t dynamixel_packet_compute_checksum(DynamixelPacket *packet);
// computes chekcsum and compares with the existing one
bool dynamixel_packet_checksum_isok(DynamixelPacket *packet);



#ifdef __cplusplus
}
#endif

