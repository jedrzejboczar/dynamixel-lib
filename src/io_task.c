#include "io_task.h"

#include <string.h>


static uint32_t max_wait_ticks(DynamixelIOTaskHandle *task,
        uint32_t n_bytes, bool is_reading);
static void maybe_send_response(DynamixelIOStatus status,
        DynamixelIORequest *request, DynamixelIOResponse *response,
        DynamixelIOTaskHandle *handle);

void dynamixel_io_task(void *arguments)
{
    DynamixelIOTaskHandle *task_handle =
        (DynamixelIOTaskHandle *) arguments;
    DynamixelIORequest request;
    DynamixelIOResponse response;
    uint32_t notification_value;
    int uart_result;

    while (1)
    {
        task_handle->transmission_state = dio_NOT_COMPLETED;
        // reinitialize response
        response.data = NULL;
        response.data_len = 0;
        response.status = dio_UNDEFINED;

        // wait forever for command to be transmitted
        BaseType_t queue_result = xQueueReceive(task_handle->request_queue,
                &request, portMAX_DELAY);
        configASSERT(queue_result == pdTRUE);
        configASSERT(request.packet != NULL);
        configASSERT(request.response_size >= 0);

        // start transmission
        uart_result = task_handle->uart_write_handle(
                dynamixel_packet_data(request.packet),
                dynamixel_packet_size(request.packet));

        if (uart_result != 0) {
            // recover from uart error
            task_handle->uart_reset_handle();
            maybe_send_response(dio_UART_WRITE_ERROR, &request, &response, task_handle);
            continue; // back to waiting
        }

        // wait for transmission end
        // do not care for how many notifications were received (should be one)
        notification_value = ulTaskNotifyTake(pdTRUE,
                max_wait_ticks(task_handle, dynamixel_packet_size(request.packet), false));

        if (notification_value == 0) {
            // recover from timeout
            task_handle->uart_reset_handle();
            maybe_send_response(dio_UART_WRITE_TIMEOUT, &request, &response, task_handle);
            continue; // back to waiting
        }

        if (task_handle->transmission_state != dio_WRITE_COMPLETED) {
            // recover from timeout
            task_handle->uart_reset_handle();
            maybe_send_response(dio_WRONG_NOTIFICATION, &request, &response, task_handle);
            continue; // back to waiting
        }

        if (request.response_size == 0) {
            // nothing to be received
            maybe_send_response(dio_OK, &request, &response, task_handle);
            continue; // back to waiting
        }

        // clear packet contents (just in case)
        // dynamixel_packet_init(request.packet, 0, 0);
        // set known values to easier check if anything was read
        uint8_t markers[] = {0xba, 0xad, 0xf0, 0x0d, 0xba, 0xad, 0xf0, 0x0d}; // baad food
        size_t markers_len = sizeof(markers) / sizeof(*markers);
        memcpy(request.packet, markers,
                sizeof(DynamixelPacket) < markers_len ? sizeof(DynamixelPacket) : markers_len );

        // receive response
        uart_result = task_handle->uart_read_handle(
                dynamixel_packet_data(request.packet),
                request.response_size);

        if (uart_result != 0) {
            // recover from uart error
            task_handle->uart_reset_handle();
            maybe_send_response(dio_UART_READ_ERROR, &request, &response, task_handle);
            continue; // back to waiting
        }

        // wait for transmission to end
        notification_value = ulTaskNotifyTake(pdTRUE,
                max_wait_ticks(task_handle, request.response_size, true));

        if (notification_value == 0) {
            // recover from timeout
            task_handle->uart_reset_handle();
            maybe_send_response(dio_UART_READ_TIMEOUT, &request, &response, task_handle);
            continue; // back to waiting
        }

        if (task_handle->transmission_state != dio_READ_COMPLETED) {
            // recover from timeout
            task_handle->uart_reset_handle();
            maybe_send_response(dio_WRONG_NOTIFICATION, &request, &response, task_handle);
            continue; // back to waiting
        }

        if (!dynamixel_packet_checksum_isok(request.packet)) {
            // recover from wrong checksum
            maybe_send_response(dio_WRONG_CHECKSUM, &request, &response, task_handle);
            continue; // back to waiting
        }

        // communication successful
        response.data = request.packet->parameters_with_checksum;
        response.data_len = dynamixel_packet_n_parameters(request.packet);
        // this should be always true (from definition)
        configASSERT(request.response_size ==
                response.data_len + DYNAMIXEL_PACKET_BASE_SIZE + 1);
        maybe_send_response(dio_OK, &request, &response, task_handle);

    }

    // the task should probably never end anyway
#if ( INCLUDE_vTaskDelete == 1 )
    vTaskDelete(NULL);
#else
    configASSERT(0);
#endif
}

/*********************************************************************************/

void dynamixel_io_task_create(DynamixelIOTaskHandle *handle,
        const char * const task_name,
        UBaseType_t task_priority,
        UBaseType_t queues_length,
        HalfDuplexUARTNonBlockingWrite uart_write_handle,
        HalfDuplexUARTNonBlockingRead uart_read_handle,
        HalfDuplexUARTReset uart_reset_handle,
        uint32_t max_wait_per_byte_us,
        uint32_t max_wait_read_delay_us)
{
    // TODO: this may be possible to implement a way to use longer queues,
    //       but now longer queue may cause a task to recive a response
    //       for requests made by other task
    configASSERT(queues_length == 1);
    // check if parameter values are ok
    configASSERT(handle != NULL);
    configASSERT(task_name != NULL);
    configASSERT(uart_write_handle != NULL);
    configASSERT(uart_read_handle != NULL);
    configASSERT(uart_reset_handle != NULL);
    // initialise user configuration fields
    handle->uart_write_handle = uart_write_handle;
    handle->uart_read_handle = uart_read_handle;
    handle->uart_reset_handle = uart_reset_handle;
    handle->max_wait_per_byte_us = max_wait_per_byte_us;
    handle->max_wait_read_delay_us = max_wait_read_delay_us;
    handle->transmission_state = dio_NOT_COMPLETED;
    // allocate rtos structures
    BaseType_t result = xTaskCreate(dynamixel_io_task,
            task_name,
            configMINIMAL_STACK_SIZE, // TODO: how much?
            (void *) handle,
            task_priority,
            &handle->task_handle);
    configASSERT(result == pdPASS);
    handle->request_queue = xQueueCreate(queues_length, sizeof(DynamixelIORequest));
    handle->response_queue = xQueueCreate(queues_length, sizeof(DynamixelIOResponse));
    configASSERT(handle->request_queue != NULL);
    configASSERT(handle->response_queue != NULL);
}


void dynamixel_io_task_notify_transmission_complete(DynamixelIOTaskHandle *dio_task_handle,
        DynamixelIOTransmissionState state)
{
    TaskHandle_t task_handle = dio_task_handle->task_handle;
    BaseType_t higher_priority_task_woken = pdFALSE;
    configASSERT(task_handle != NULL);
    dio_task_handle->transmission_state = state;
    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR(task_handle, &higher_priority_task_woken);
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
       should be performed to ensure the interrupt returns directly to the highest
       priority task.  The macro used for this purpose is dependent on the port in
       use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

bool dynamixel_io_send_request(DynamixelIOTaskHandle *task_handle,
        DynamixelPacket *packet, int response_size, bool ignore_response)
{
    DynamixelIORequest request = {
        .packet = packet,
        .response_size = response_size,
        .ignore_response = ignore_response
    };
    BaseType_t result = xQueueSendToBack(task_handle->request_queue,
            &request,
            portMAX_DELAY);
            // 2 * portTICK_PERIOD_MS(task_handle->max_wait_time_ms));
    // TODO: ^ should it wait portMAX_DELAY?
    //         it must be more than used in ulTaskNotifyTake !!!
    return result == pdPASS;
}

bool dynamixel_io_wait_response(DynamixelIOTaskHandle *task_handle,
        DynamixelIOResponse *response)
{
    BaseType_t result = xQueueReceive(task_handle->response_queue,
            response,
            portMAX_DELAY);
            // 2 * portTICK_PERIOD_MS(task_handle->max_wait_time_ms));
    // TODO: ^ should it wait portMAX_DELAY?
    //         it must be more than used in ulTaskNotifyTake !!!
    return result == pdTRUE;
}

static uint32_t max_wait_ticks(DynamixelIOTaskHandle *task,
        uint32_t n_bytes, bool is_reading)
{
    uint32_t max_wait_us = n_bytes * task->max_wait_per_byte_us;
    if (is_reading)
        max_wait_us += task->max_wait_read_delay_us;
    uint32_t divider = portTICK_PERIOD_MS * 1000; // divde by 1000 to convert microsec to milisec
    uint32_t max_wait_ticks = ((max_wait_us - 1) / divider) + 1; // ceiling division (rounds up)
    return max_wait_ticks;
}

static void maybe_send_response(DynamixelIOStatus status,
        DynamixelIORequest *request, DynamixelIOResponse *response,
        DynamixelIOTaskHandle *handle)
{
    if (!request->ignore_response) {
        response->status = status;
        BaseType_t queue_result = xQueueSendToBack(handle->response_queue,
                response, portMAX_DELAY);
        configASSERT(queue_result == pdTRUE);
    }
}


