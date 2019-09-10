#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Implementation of IO communication with Dynamixel servos using FreeRTOS.
 *
 * Communication is generic, and a peripheral driver functions must be provided.
 * Dynamixel servos use half-duplex UART and many servos can be connected to
 * the same UART line. For each of such groups one task should be created.
 *
 * Communication implementation is similar to:
 *   https://www.freertos.org/RTOS_Task_Notification_As_Binary_Semaphore.html
 *
 * Usage:
 * 0. Configure UART, implement UART communication functions
 *      (uart_write_handle, uart_read_handle, uart_reset_handle)
 *    along with a call to dynamixel_io_task_notify_transmission_complete()
 *    in transfer completion interrupt.
 * 1. Initialize DynamixelIOTaskHandle structure using dynamixel_io_task_create(),
 *    this also creates FreeRTOS task and queues.
 * 2. Create and fill DynamixelPacket, then send request to request_queue:
 *       dynamixel_io_send_request(...)
 * 3. (!) If request.ignore_response == false, create DynamixelIOResponse and wait:
 *       dynamixel_io_wait_response(...)
 *    or else the task will fill up response queue and hang until it is cleared!
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "dynamixel.h"

/*
 * UART communication function signatures that have to be implemented by user.
 * They should start non-blocking data transmition/reception,
 * then, in a corresponding interrupt routine, the should call
 *  dynamixel_io_task_notify_transmission_complete(task_handle)
 *
 * Each function should return 0 on success (anythign else on error).
 *
 * If the same UART has to be used by many tasks, then locking logic should
 * be implemented in these functions by the user.
 */
typedef int (*HalfDuplexUARTNonBlockingWrite)(uint8_t *data, size_t data_len);
typedef int (*HalfDuplexUARTNonBlockingRead)(uint8_t *data, size_t data_len);
typedef int (*HalfDuplexUARTReset)(void);


typedef enum {
    dio_WRITE_COMPLETED,
    dio_READ_COMPLETED,
    dio_NOT_COMPLETED
} DynamixelIOTransmissionState;

/*
 * Structure representing IO task.
 * Task configuration should be set before creatiog the task,
 * while task and queues handles are used for:
 *  - calling dynamixel_io_task_notify_transmission_complete()
 *  - waiting for response with xQueueReceive(handle.response_queue, ...)
 */
typedef struct {
    // Handles
    TaskHandle_t task_handle;
    QueueHandle_t request_queue;
    QueueHandle_t response_queue;
    // Task cofiguration
    HalfDuplexUARTNonBlockingWrite uart_write_handle;
    HalfDuplexUARTNonBlockingRead uart_read_handle;
    HalfDuplexUARTReset uart_reset_handle;
    // timing constraints: per byte and read delay (servo waits before sending response)
    // FIXME: reading seems to require much more time (needed overall 3ms for 8 bytes at BR=57600b/s)
    uint32_t max_wait_per_byte_us;   // usually =~ ( 1 / (baud_rate / (8+1)) ) * 10^6
    uint32_t max_wait_read_delay_us; // depends on dynamixel Return Delay Time (default 500us)
    // internal variable for verifying proper task notification
    DynamixelIOTransmissionState transmission_state;
} DynamixelIOTaskHandle;

typedef enum {
    dio_OK = 0,
    dio_UART_WRITE_ERROR,     // error returned by uart_write_handle
    dio_UART_READ_ERROR,      // error returned by uart_read_handle
    dio_UART_WRITE_TIMEOUT,   // task not notified for more than maximum wait time
    dio_UART_READ_TIMEOUT,    // task not notified for more than maximum wait time
    dio_WRONG_NOTIFICATION,   // received notification from wrong transmission type
    dio_WRONG_CHECKSUM,       // received response, but checksum is wrong
    dio_UNDEFINED,            // should never happen
} DynamixelIOStatus;

typedef struct {
    DynamixelPacket *packet;  // pointer to already created packet
    int response_size;        // expected size of response (0 for no response)
    bool ignore_response;     // if true, than no DynamixelIOResponse will be sent,
                              // useful when task does not want to wait for response_queue
} DynamixelIORequest;

typedef struct {
    DynamixelIOStatus status; // status of communication, data is valid only for dio_OK
    uint8_t *data;            // points to received data
                              // (start of request.packet.parameters_with_checksum)
    int data_len;             // length of data
} DynamixelIOResponse;


void dynamixel_io_task(void *arguments);
void dynamixel_io_task_create(DynamixelIOTaskHandle *handle,
        const char * const task_name,
        UBaseType_t task_priority,
        UBaseType_t queues_length,
        HalfDuplexUARTNonBlockingWrite uart_write_handle,
        HalfDuplexUARTNonBlockingRead uart_read_handle,
        HalfDuplexUARTReset uart_reset_handle,
        uint32_t max_wait_per_byte_us,
        uint32_t max_wait_read_delay_us);
void dynamixel_io_task_notify_transmission_complete(DynamixelIOTaskHandle *dio_task_handle,
        DynamixelIOTransmissionState state);
// wrappers around xQueueSendToBack/xQueueReceive; return false on queue timeout
bool dynamixel_io_send_request(DynamixelIOTaskHandle *task_handle,
        DynamixelPacket *packet, int response_size, bool ignore_response);
bool dynamixel_io_wait_response(DynamixelIOTaskHandle *task_handle,
        DynamixelIOResponse *response);

#ifdef __cplusplus
}
#endif

