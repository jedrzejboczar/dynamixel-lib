# dynamixel-lib

This is a simple library for communication with Dynamixel AX-12 servos
(also confirmed for AX-18, just check if memory map and protocol are the same for other models).
Calling it a library may actually be a little too much, but anyway it is a usable piece of code,
so someone may save some work by using it instead of writing from scratch.

The library doesn't have to be used as a whole.
It consists of parts that have different dependencies and different level of abstraction.
Depending on available resources only some parts can be used.
The CMake configuration provided is really minimal, so for real use it should probably be extended.

## Modules

Functions for abstraction over dynamixel packets:
- dependencies:
    - this part doesn't have any real dependencies (just some reasonable C standard)
- headers:
    - defines.h - macros for different dynamixel constants (memory adresses etc.)
    - packet.h - low level definition of DynamixelPacket
    - dynamixel.h - higher level abstractions for assembling packets

FreeRTOS task for communication over single UART line
- dependencies:
    - FreeRTOS
- headers:
    - io_task.h

Abstraction over group of servos connected to single UART, makes writing/reading multiple servos really convenient:
- dependencies:
    - more FreeRTOS (semphr.h)
    - freertos_cpp/lock_by_proxy.h from this project (TODO: add it to this repository!), it allows for quite convenient and robust locking of the whole class
- headers:
    - servo_group.h

One-use functions for discovering dynamixel servos' IDs etc. (for debug usage, inefficient and heavy)
- dependencies:
    - user must provide a function for printing
- header:
    - discovery_utils.h


## Tests

In *test/* there are some tests of low-level functionalities written in [cmocka](https://api.cmocka.org/).
