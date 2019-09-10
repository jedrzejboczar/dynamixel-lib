#include "servo_group.h"
#include <string.h>

namespace Dynamixel {


/*** Servo ********************************************************************/
Servo::Servo(uint8_t id): servo_id(id) {
    configASSERT(id != DYNAMIXEL_BROADCASTING_ID);
}

void Servo::prepare_u8(uint8_t address, uint8_t value) {
    select();
    reg_address = address;
    is_2_bytes = false;
    data_buffer[0] = value;
}

void Servo::prepare_u16(uint8_t address, uint16_t value) {
    select();
    reg_address = address;
    is_2_bytes = true;
    // lower byte first
    data_buffer[0] = value & 0xff;
    data_buffer[1] = value >> 8;
}

void Servo::select(bool value) {
    selected = value;
}

uint8_t Servo::id() {
    return servo_id;
}

uint8_t Servo::address() {
    return reg_address;
}

uint8_t Servo::data_u8() {
    return data_buffer[0];
}

uint16_t Servo::data_u16() {
    uint16_t value = (static_cast<uint16_t>(data_buffer[1]) << 8) | (data_buffer[0] & 0xff);
    return value;
}

const uint8_t *Servo::data() {
    return data_buffer;
}

int Servo::data_length() {
    return is_2_bytes ? 2 : 1;
}

bool Servo::is_selected() {
    return selected;
}


/*** ServoGroup ***************************************************************/

ServoGroup::ServoGroup(DynamixelIOTaskHandle *task_handle,
        Servo *servos, int n_servos):
    task_handle(task_handle), servos(servos), n_servos(n_servos)
{
    configASSERT(this->n_servos > 0);
    configASSERT(this->servos != nullptr);
    configASSERT(this->task_handle != nullptr);

    // FIXME: requires packets large enough to perform 16-bit sync-write
    // for the whole group, bytes: 2_always + N * (1_id + 2_16bitdata)
    // (this simplifies the design, and support may be implemented later)
    configASSERT(DYNAMIXEL_MAX_N_PARAMETERS >= 2 + n_servos * (1 + 2));

    // take the mutex, we will give it away only after initialise() :D
    // can be run without scheduler if xTicksToWait == 0
    configASSERT(take(0));
}

bool ServoGroup::is_initialised() {
    return initialised;
}

bool ServoGroup::initialise() {
    // ignore subsequent calls
    if (initialised)
        return true;

    // ping each servo to check if we can communicate
    for (int i = 0; i < n_servos; i++) {
        bool is_alive = false;
        // check more than once, not to be over-sensitive
        for (int j = 0; j < 3; j++) {
            is_alive = is_alive || ping_servo(i);
            if (is_alive) break;
        }
        if (!is_alive) {
            return false;
        }
    }

    // check model numbers!
    prepare_all_u16(DYNAMIXEL_MODEL_NUMBER_L);
    if (!read_selected())
        return false;
    // these are the only ones verified
    for (int i = 0; i < n_servos; i++) {
        configASSERT(servos[i].data_u16() == DYNAMIXEL_AX12_MODEL_NUMBER
                || servos[i].data_u16() == DYNAMIXEL_AX18_MODEL_NUMBER);
    }

    // set defaults:

    // make sure torque is disabled
    prepare_all_u8(DYNAMIXEL_TORQUE_ENABLE, 0);
    if (!sync_selected())
        return false;

    // set sensible alarm LED and alarm shutdown (default overheating error only)
    prepare_all_u8(DYNAMIXEL_ALARM_LED, // all for now
            DYNAMIXEL_ERROR_INSTRUCTION_MASK |
            DYNAMIXEL_ERROR_OVERLOAD_MASK    |
            DYNAMIXEL_ERROR_CHECKSUM_MASK    |
            DYNAMIXEL_ERROR_RANGE_MASK       |
            DYNAMIXEL_ERROR_OVERHEATING_MASK |
            DYNAMIXEL_ERROR_ANGLE_LIMIT_MASK |
            DYNAMIXEL_ERROR_INPUT_VOLTAGE_MASK);
    if (!sync_selected())
        return false;
    prepare_all_u8(DYNAMIXEL_ALARM_SHUTDOWN,
            DYNAMIXEL_ERROR_OVERHEATING_MASK | DYNAMIXEL_ERROR_INPUT_VOLTAGE_MASK);
    if (!sync_selected())
        return false;

    // return delay time?
    // clockwise&counter-clockwise angle limits
    //
    // max torque?
    // status return level
    // alarm LED conditions
    // alar shutdown conditions
    //
    // disable torque
    // blink LED
    // compilance margins and slopes
    //
    // torque limit
    //
    // assert registered instuction = 0
    //
    //

    // now, when everything went well we can give mutex
    // to allow usage of ServoGroup
    configASSERT(give());
    initialised = true;

    return true;
}

bool ServoGroup::sync_selected(bool unselect) {
    // TODO: implement situtations (using reg-write + action?):
    //  - some servos have different lengths of data to be sent
    //  - some servos have different adresses to be sent to
    //  - not enough space in packet to send all data in one sync-write

    // // find requested data length
    // int data_len = servos[0].data_length();
    // uint8_t address = servos[0].address();
    // // as for now all servos have to have the same data length
    // // as for now all servos have to have the same address too...
    // for (int i = 0; i < n_servos; i++)
    //     if (servos[i].data_length() != data_len
    //             || servos[i].address() != address)
    //         return false;
    // configASSERT(data_len == 1 || data_len == 2);

    auto first_selected = std::find_if(servos, servos + n_servos,
            [](Servo &s){ return s.is_selected(); });
    configASSERT(first_selected != servos + n_servos);

    int data_len = first_selected->data_length();
    uint8_t address = first_selected->address();

    bool are_all = std::all_of(servos, servos + n_servos, [data_len, address](Servo &s) {
            if (!s.is_selected())
                return true;
            return s.data_length() == data_len && s.address() == address;
        });

    if (!are_all)
        return false;

    configASSERT(data_len == 1 || data_len == 2);


    // Servo *first_selected = nullptr;
    // for (int i = 0; i < n_servos; i++) {
    //     if (servos[i].is_selected()) {
    //         first_selected = &servos[i];
    //         break;
    //     }
    // }
    // configASSERT(first_selected != nullptr);
    //
    // int data_len = first_selected->data_length();
    // uint8_t address = first_selected->address();
    //
    // for (int i = 0; i < n_servos; i++) {
    //     if (servos[i].is_selected())
    //         if (servos[i].data_length() != data_len
    //                 || servos[i].address() != address)
    //             return false;
    // }
    //
    // configASSERT(data_len == 1 || data_len == 2);




    // assemble the packet
    int init_status = dynamixel_prepare_sync_write_init(&packet, address, data_len);
    // it would be sad if there was not enough space even for init, but whatever...
    if (init_status != 0)
        return false;

    // FIXME: not really needed but better check 2 times than not at all
    // calculate if it is possible to send data to all in one sync-write
    //                 N * (id + data) + checksum
    int space_needed = n_servos * (1 + data_len) + 1;
    int space_remaining = dynamixel_packet_space_remaining(&packet);
    configASSERT(space_remaining >= space_needed);

    // add data from servos
    for (int i = 0; i < n_servos; i++) {
        Servo &servo = servos[i];
        if (servo.is_selected()) {
            int result = dynamixel_prepare_sync_write_add_next(&packet,
                    servo.id(), servo.data());
            if (result != 0)
                return false;
        }
    }

    // start transfer
    int response_size = dynamixel_prepare_sync_write_end(&packet);
    // assert that we eariler calculated required space correctly
    if (response_size != 0)
        return false;

    bool is_ok = dynamixel_io_send_request(task_handle,
            &packet, response_size, false);
    if (!is_ok)
        return false;

    DynamixelIOResponse response;
    is_ok = dynamixel_io_wait_response(task_handle, &response);
    if (!is_ok || response.status != dio_OK)
        return false;

    if (unselect)
        select_all(false);

    return true;
}

bool ServoGroup::read_selected(bool unselect) {
    // read each servo separately
    for (int i = 0; i < n_servos; i++) {
        // TODO: allows only reading and writing 1 or 2 bytes
        int len = servos[i].data_length();
        configASSERT(len == 1 || len == 2);
        // prepare packet
        int response_size;
        if (len == 1)
            response_size = dynamixel_prepare_read_register_u8(&packet,
                    servos[i].id(), servos[i].address());
        else
            response_size = dynamixel_prepare_read_register_u16(&packet,
                    servos[i].id(), servos[i].address());

        // send and wait for response
        bool is_ok = dynamixel_io_send_request(task_handle,
                &packet, response_size, false);
        if (!is_ok) return false;
        DynamixelIOResponse response;
        is_ok = dynamixel_io_wait_response(task_handle, &response);
        if (!is_ok || response.status != dio_OK)
            return false;

        // if response was ok, then move the data to Servo
        servos[i].data_buffer[0] = response.data[0];
        if (len == 2)
            servos[i].data_buffer[1] = response.data[1];
        // save last error values
        servos[i].last_error = packet.error;
    }
    if (unselect)
        select_all(false);
    return true;
}

bool ServoGroup::read_one(int num, uint8_t *into, uint8_t start_address, int n_bytes) {
    int response_size = dynamixel_prepare_read(&packet,
            servos[num].id(), start_address, n_bytes);

    bool is_ok = dynamixel_io_send_request(task_handle,
            &packet, response_size, false);
    if (!is_ok) return false;
    DynamixelIOResponse response;
    is_ok = dynamixel_io_wait_response(task_handle, &response);
    if (!is_ok || response.status != dio_OK)
        return false;

    // copy data to destination
    memcpy(into, response.data, response.data_len);
    return true;
}

bool ServoGroup::ping_servo(int num) {
    configASSERT(num >= 0 && num < n_servos);

    int response_size = dynamixel_prepare_ping(&packet, servos[num].id());
    configASSERT(response_size >= 0); // ping has to have response

    bool is_ok = dynamixel_io_send_request(task_handle,
            &packet, response_size, false);
    if (!is_ok) return false;

    DynamixelIOResponse response;
    is_ok = dynamixel_io_wait_response(task_handle, &response);
    return is_ok && response.status == dio_OK;
}




// for writing the same data to many servos
void ServoGroup::prepare_all_u8(uint8_t address, uint8_t value) {
    for (int i = 0; i < n_servos; i++)
        servos[i].prepare_u8(address, value);
}

void ServoGroup::prepare_all_u16(uint8_t address, uint16_t value) {
    for (int i = 0; i < n_servos; i++)
        servos[i].prepare_u16(address, value);
}

void ServoGroup::prepare_selected_u8(uint8_t address, uint8_t value) {
    for (int i = 0; i < n_servos; i++)
        if (servos[i].is_selected())
            servos[i].prepare_u8(address, value);
}

void ServoGroup::prepare_selected_u16(uint8_t address, uint16_t value) {
    for (int i = 0; i < n_servos; i++)
        if (servos[i].is_selected())
            servos[i].prepare_u16(address, value);
}


void ServoGroup::select_all(bool value) {
    for (int i = 0; i < n_servos; i++)
       servos[i].select(value);
}

Servo& ServoGroup::operator[] (int num) {
    configASSERT(num >= 0 && num < n_servos);
    return servos[num];
}

int ServoGroup::len() {
    return n_servos;
}




} // namespace Dynamixel
