#include "discovery_utils.h"

static bool ping_servo(DynamixelIOTaskHandle *io_task, DynamixelPacket *packet, uint8_t servo_id);

void dynamixel_servos_discovery(DynamixelIOTaskHandle *io_task,
        int (*write_function)(char *ptr, int len))
{
    DynamixelPacket packet;

    if (write_function == NULL)
        return;

    const char *separator = "\n----------------------------------------------\n";
    write_function((char *) separator, strlen(separator));
    vTaskDelay(pdMS_TO_TICKS(5));

    // iterate over all possible servo IDs
    for (uint8_t i = 0; i < DYNAMIXEL_BROADCASTING_ID; ++i) {

        bool found = false;
        for (int j = 0; j < 3; j++) {
            found = found || ping_servo(io_task, &packet, i);
            if (found) break;
        }
        if (!found)
            continue;

        // if servo was found, get all the needed information
        bool is_ok;
        int response_size;
        DynamixelIOResponse response;

        uint16_t model_number = 0;
        bool model_number_ok = false;
        uint8_t firmware_version = 0;
        bool firmware_version_ok = false;
        uint8_t baud_rate = 0;
        bool baud_rate_ok = false;
        uint8_t return_delay_time = 0;
        bool return_delay_time_ok = false;

        response_size = dynamixel_prepare_read_register_u16(&packet,
                i, DYNAMIXEL_MODEL_NUMBER_L);
        is_ok = dynamixel_io_send_request(io_task, &packet, response_size, false);
        if (is_ok)
            is_ok = dynamixel_io_wait_response(io_task, &response);
        if (is_ok && response.status == dio_OK) {
            model_number_ok = true;
            model_number = ((uint16_t) (response.data[1]) << 8) | (response.data[0] & 0xff);
        }

        // to use a loop
        uint8_t addresses[] = {DYNAMIXEL_VERSION, DYNAMIXEL_BAUD_RATE, DYNAMIXEL_RETURN_DELAY_TIME};
        uint8_t *registers[] = {&firmware_version, &baud_rate, &return_delay_time};
        bool *oks[] = {&firmware_version_ok, &baud_rate_ok, &return_delay_time_ok};

        for (size_t j = 0; j < sizeof(registers) / sizeof(*registers); j++) {
            response_size = dynamixel_prepare_read_register_u8(&packet, i, addresses[j]);
            is_ok = dynamixel_io_send_request(io_task, &packet, response_size, false);
            if (is_ok)
                is_ok = dynamixel_io_wait_response(io_task, &response);
            if (is_ok && response.status == dio_OK) {
                *oks[j] = true;
                *registers[j] = response.data[0];
            }
        }

        // print info about servos
        char msg_buffer[150] = {0};

        int n_printed = snprintf(msg_buffer, sizeof(msg_buffer) / sizeof(*msg_buffer),
                "Found servo with ID %d\n"
                "-> model number = 0x%02x %s\n"
                "-> firmware ver = 0x%02x %s\n"
                "-> baud rate    = 0x%02x %s\n"
                "-> return delay = 0x%02x %s\n"
                , i,
                model_number, model_number_ok ? "" : "ERROR",
                firmware_version, firmware_version_ok ? "" : "ERROR",
                baud_rate, baud_rate_ok ? "" : "ERROR",
                return_delay_time, return_delay_time_ok ? "" : "ERROR");

        configASSERT(n_printed > 0);

        // if this fails, then increase buffer size
        configASSERT((unsigned int) n_printed < ( sizeof(msg_buffer) / sizeof(*msg_buffer) + 1));

        write_function(msg_buffer, strlen(msg_buffer));

        // this is for printing with ITM
        // (may be ommitted if printing works well without it)
        vTaskDelay(pdMS_TO_TICKS(15));
    }

    write_function((char *) separator, strlen(separator));
    vTaskDelay(pdMS_TO_TICKS(5));
}

static bool ping_servo(DynamixelIOTaskHandle *io_task, DynamixelPacket *packet, uint8_t servo_id) {
    int response_size = dynamixel_prepare_ping(packet, servo_id);
    configASSERT(response_size >= 0); // ping has to have response
    // send ping
    bool is_ok = dynamixel_io_send_request(io_task, packet, response_size, false);
    if (!is_ok) return false;

    // wait for response
    DynamixelIOResponse response;
    is_ok = dynamixel_io_wait_response(io_task, &response);

    return is_ok && response.status == dio_OK;
}

