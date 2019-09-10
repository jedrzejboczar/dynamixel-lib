#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "io_task.h"

/*
 * Function that performs search for servos connected to UART.
 * This function is intended for debugging usage.
 * It must be used from within a task (FreeRTOS scheduler running).
 *
 * NOTE: task may need quite a lot stack size!
 *
 * When it finds a servo, it will print information about it
 * using the given write function, the function should return
 * number of charaters written.
 *
 * UART baud rate must be the same as it is in servos,
 * to find every possible servo we would have to iterate over all
 * possible baud rate values.
 */
void dynamixel_servos_discovery(DynamixelIOTaskHandle *io_task,
        int (*write_function)(char *ptr, int len));


#ifdef __cplusplus
}
#endif

