#pragma once

#include <algorithm>

#include "freertos_cpp/mutex.h"
#include "io_task.h"


namespace Dynamixel {

/*
 * Class that represents a physical servo with given ID.
 * This class should be used with ServoGroup, it allows selecting servos
 * that will be written later and is used for storing data for
 * reading/writing 1 or 2 bytes.
 */
class Servo {
public:
    Servo(uint8_t id);

    /*
     * Used to choose servo address to be wrritten/read.
     * Automatically selects the servo.
     * When writing we want to specify the value to be written.
     * When reading the value is ignored anyway (no need to pass it).
     *
     * There are two methods as uint8/uint16 calls may be ambigious.
     */
    void prepare_u8(uint8_t address, uint8_t value = 0);
    void prepare_u16(uint8_t address, uint16_t value = 0);
    void select(bool value=true);

    // accessors
    uint8_t id();
    uint8_t address();
    uint8_t data_u8();
    uint16_t data_u16();
    const uint8_t *data();
    int data_length();
    bool is_selected();

    // TODO: to be added:
    // - id changing - requires some special checks or we may loose servo's id
private:
    // needed for reading data into Servo.data_buffer
    friend class ServoGroup;

    uint8_t servo_id;
    uint8_t last_error;  // value of packet.error from last reading operation
    uint8_t reg_address;
    uint8_t data_buffer[2];
    // avoid taking too much space (Servos are to be stored in an array): subsequent
    // bit-fields of the same type are connected (8 times bool: 1 takes one byte)
    bool is_2_bytes: 1;
    bool selected: 1;
};

/*
 * Class that represents a group of servos connected to the same UART line.
 * This line is managed by the task given by DynamixelIOTaskHandle.
 *
 * Constructor does not perform any communication (as it actually would be very
 * inconvenient), so in order to use ServoGroup, it has to be manually initialised
 * using initialise() (this can be called only when FreeRTOS scheduler is running).
 *
 * WARNING: Results of using ServoGroup before initialization are undefined, except using
 * take() - then if will block on mutex until initialization is performed (thus
 * take() should always be used in RTOS, see below).
 *
 * The usage idea is to perform operations in two steps: first prepare each servo
 * that is to be written/read, and then perform the actual communication.
 * To make it possible in multi-threaded application we have to be able to prevent
 * access to this class between preparation and communication. This is achieved
 * by deriving from Mutex, which allows to take()/give() access to the class.
 * Also Lock allows to automatic scoped give() (gives in Lock's destructor).
 *
 */
class ServoGroup: public Mutex {
public:
    /* Initialise the structure (no initial communcation,
     * may be created before FreeRTOS scheduler is started)
     * To use this class initialise() must be called when scheduler is running,
     * this is counter-C++, but it is problematic when the class cannot
     * be instantiated before starting of the sceduler. */
    ServoGroup(DynamixelIOTaskHandle *task_handle,
            Servo *servos, int n_servos);

    /* Perform initial communication, write default values
     * (requires FreeRTOS scheduler running) */
    bool initialise();
    bool is_initialised();

    // writing to servos through uart task,
    // by default unselects all servos after operation
    bool sync_selected(bool unselect=true);
    bool read_selected(bool unselect=true);
    // bool write_servo(int num, uint8_t address, );
    bool ping_servo(int num);

    // TODO: add reading (or writing) more data from one servo
    bool read_one(int num, uint8_t *into, uint8_t start_address, int n_bytes);

    void select_all(bool value=true);

    // convenience methods for preparing many servos with the same data
    // for reading value can be omitted
    void prepare_all_u8(uint8_t address, uint8_t value = 0);
    void prepare_all_u16(uint8_t address, uint16_t value = 0);
    void prepare_selected_u8(uint8_t address, uint8_t value = 0);
    void prepare_selected_u16(uint8_t address, uint16_t value = 0);

    // getters for servo array
    Servo& operator[] (int num);
    int len();

private:
    DynamixelIOTaskHandle *task_handle; // task to handle comunication over UART
    DynamixelPacket packet;             // structure for storing UART packets
    Servo *servos;                      // pointer to prealocated array of DynamixelServo
    const int n_servos;                       // number of servos in the group
    bool initialised;                   // specifies wheather initialise() has been called
};




} // namespace Dynamixel
