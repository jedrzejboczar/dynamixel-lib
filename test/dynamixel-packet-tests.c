#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "dynamixel_packet_tests.h"
#include "dynamixel_tests.h"


int main(void) {
    return run_dynamixel_tests() + run_dynamixel_packet_tests();
}

