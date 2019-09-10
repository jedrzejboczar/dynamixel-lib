#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Model number for AX12 servos */
#define DYNAMIXEL_AX12_MODEL_NUMBER           0x0c
#define DYNAMIXEL_AX18_MODEL_NUMBER           0x12

/* --- MEMORY ADDRESSING --- */
// EEPROM area
#define DYNAMIXEL_MODEL_NUMBER_L              0x00
#define DYNAMIXEL_MODOEL_NUMBER_H             0x01
#define DYNAMIXEL_VERSION                     0x02
#define DYNAMIXEL_ID                          0x03
#define DYNAMIXEL_BAUD_RATE                   0x04
#define DYNAMIXEL_RETURN_DELAY_TIME           0x05
#define DYNAMIXEL_CW_ANGLE_LIMIT_L            0x06
#define DYNAMIXEL_CW_ANGLE_LIMIT_H            0x07
#define DYNAMIXEL_CCW_ANGLE_LIMIT_L           0x08
#define DYNAMIXEL_CCW_ANGLE_LIMIT_H           0x09
#define DYNAMIXEL_SYSTEM_DATA2                0x0a
#define DYNAMIXEL_LIMIT_TEMPERATURE           0x0b
#define DYNAMIXEL_DOWN_LIMIT_VOLTAGE          0x0c
#define DYNAMIXEL_UP_LIMIT_VOLTAGE            0x0d
#define DYNAMIXEL_MAX_TORQUE_L                0x0e
#define DYNAMIXEL_MAX_TORQUE_H                0x0f
#define DYNAMIXEL_RETURN_LEVEL                0x10
#define DYNAMIXEL_ALARM_LED                   0x11
#define DYNAMIXEL_ALARM_SHUTDOWN              0x12
#define DYNAMIXEL_OPERATING_MODE              0x13
#define DYNAMIXEL_DOWN_CALIBRATION_L          0x14
#define DYNAMIXEL_DOWN_CALIBRATION_H          0x15
#define DYNAMIXEL_UP_CALIBRATION_L            0x16
#define DYNAMIXEL_UP_CALIBRATION_H            0x17
// RAM area
#define DYNAMIXEL_TORQUE_ENABLE               0x18
#define DYNAMIXEL_LED                         0x19
#define DYNAMIXEL_CW_COMPLIANCE_MARGIN        0x1a
#define DYNAMIXEL_CCW_COMPLIANCE_MARGIN       0x1b
#define DYNAMIXEL_CW_COMPLIANCE_SLOPE         0x1c
#define DYNAMIXEL_CCW_COMPLIANCE_SLOPE        0x1d
#define DYNAMIXEL_GOAL_POSITION_L             0x1e
#define DYNAMIXEL_GOAL_POSITION_H             0x1f
#define DYNAMIXEL_GOAL_SPEED_L                0x20
#define DYNAMIXEL_GOAL_SPEED_H                0x21
#define DYNAMIXEL_TORQUE_LIMIT_L              0x22
#define DYNAMIXEL_TORQUE_LIMIT_H              0x23
#define DYNAMIXEL_PRESENT_POSITION_L          0x24
#define DYNAMIXEL_PRESENT_POSITION_H          0x25
#define DYNAMIXEL_PRESENT_SPEED_L             0x26
#define DYNAMIXEL_PRESENT_SPEED_H             0x27
#define DYNAMIXEL_PRESENT_LOAD_L              0x28
#define DYNAMIXEL_PRESENT_LOAD_H              0x29
#define DYNAMIXEL_PRESENT_VOLTAGE             0x2a
#define DYNAMIXEL_PRESENT_TEMPERATURE         0x2b
#define DYNAMIXEL_REGISTERED_INSTRUCTION      0x2c
#define DYNAMIXEL_PAUSE_TIME                  0x2d
#define DYNAMIXEL_MOVING                      0x2e
#define DYNAMIXEL_LOCK                        0x2f
#define DYNAMIXEL_PUNCH_L                     0x30
#define DYNAMIXEL_PUNCH_H                     0x31

/* --- Instructions --- */
#define DYNAMIXEL_INST_PING                   0x01
#define DYNAMIXEL_INST_READ                   0x02
#define DYNAMIXEL_INST_WRITE                  0x03
#define DYNAMIXEL_INST_REG_WRITE              0x04
#define DYNAMIXEL_INST_ACTION                 0x05
#define DYNAMIXEL_INST_RESET                  0x06
#define DYNAMIXEL_INST_DIGITAL_RESET          0x07
#define DYNAMIXEL_INST_SYSTEM_READ            0x0C
#define DYNAMIXEL_INST_SYSTEM_WRITE           0x0D
#define DYNAMIXEL_INST_SYNC_WRITE             0x83
#define DYNAMIXEL_INST_SYNC_REG_WRITE         0x84

/* Broadcasting ID */
#define DYNAMIXEL_BROADCASTING_ID             0xFE

/* Baud rates */
#define DYNAMIXEL_BAUD_RATE_1000000           0x01
#define DYNAMIXEL_BAUD_RATE_500000            0x03
#define DYNAMIXEL_BAUD_RATE_400000            0x04
#define DYNAMIXEL_BAUD_RATE_250000            0x07
#define DYNAMIXEL_BAUD_RATE_200000            0x09
#define DYNAMIXEL_BAUD_RATE_115200            0x10
#define DYNAMIXEL_BAUD_RATE_57600             0x22
#define DYNAMIXEL_BAUD_RATE_19200             0x67
#define DYNAMIXEL_BAUD_RATE_9600              0xcf

/* Status response levels */
#define DYNAMIXEL_STATUS_RESPONSE_NEVER       0
#define DYNAMIXEL_STATUS_RESPONSE_READ_DATA   1
#define DYNAMIXEL_STATUS_RESPONSE_ALWAYS      2

/* Bit values of dynamixel packet errors */
#define DYNAMIXEL_ERROR_INSTRUCTION_MASK            (1 << 6)
#define DYNAMIXEL_ERROR_OVERLOAD_MASK               (1 << 5)
#define DYNAMIXEL_ERROR_CHECKSUM_MASK               (1 << 4)
#define DYNAMIXEL_ERROR_RANGE_MASK                  (1 << 3)
#define DYNAMIXEL_ERROR_OVERHEATING_MASK            (1 << 2)
#define DYNAMIXEL_ERROR_ANGLE_LIMIT_MASK            (1 << 1)
#define DYNAMIXEL_ERROR_INPUT_VOLTAGE_MASK          (1 << 0)
#define DYNAMIXEL_IS_INSTRUCTION_ERROR(error)      (((error) & DYNAMIXEL_ERROR_INSTRUCTION_MASK) != 0)
#define DYNAMIXEL_IS_OVERLOAD_ERROR(error)         (((error) & DYNAMIXEL_ERROR_OVERLOAD_MASK) != 0)
#define DYNAMIXEL_IS_CHECKSUM_ERROR(error)         (((error) & DYNAMIXEL_ERROR_CHECKSUM_MASK) != 0)
#define DYNAMIXEL_IS_RANGE_ERROR(error)            (((error) & DYNAMIXEL_ERROR_RANGE_MASK) != 0)
#define DYNAMIXEL_IS_OVERHEATING_ERROR(error)      (((error) & DYNAMIXEL_ERROR_OVERHEATING_MASK) != 0)
#define DYNAMIXEL_IS_ANGLE_LIMIT_ERROR(error)      (((error) & DYNAMIXEL_ERROR_ANGLE_LIMIT_MASK) != 0)
#define DYNAMIXEL_IS_INPUT_VOLTAGE_ERROR(error)    (((error) & DYNAMIXEL_ERROR_INPUT_VOLTAGE_MASK) != 0)

/* Return delay time conversions between value in register and microseconds */
#define DYNAMIXEL_RETURN_DELAY_TIME_FROM_US(us)    ((us) / 2)
#define DYNAMIXEL_RETURN_DELAY_TIME_TO_US(value)   (2 * (value))

/* Directions of turn
 * directions:  0 - counter-clockwise,  1 - clockwise */
#define DYNAMIXEL_DIRECTION_CCW            0
#define DYNAMIXEL_DIRECTION_CW             1

/* Angle conversions
 * Internally angles 0-300 deg are represented as 0-1023
 * (functions for conversion in dynamixel.h) */
#ifndef M_PI
#   define M_PI 3.141592653589793
#endif
#ifndef RAD2DEG
#   define RAD2DEG(rad)           ((rad) * 180 / (float) (M_PI))
#endif
#ifndef DEG2RAD
#   define DEG2RAD(deg)           ((deg) * (float) (M_PI) / 180)
#endif
#define DYNAMIXEL_MAX_ANGLE_DEG   300
#define DYNAMIXEL_MAX_ANGLE_RAD   DEG2RAD(DYNAMIXEL_MAX_ANGLE_DEG)
#define DYNAMIXEL_MAX_ANGLE_INT   1023

/* Speed conversions
 * Setting 0 disables any speed control (servos use maximum possible speed)
 * max speed for 0x3ff is 114 RPM = 1.9 RPS = 684 deg/s ~= 11.94 rad/s
 * TODO: check those macros (float-int problems?) */
#define DYNAMIXEL_SPEED_NO_SPEED_CONTROL   0
#define DYNAMIXEL_SPEED_MAX_VALUE          0x3ff
#define DYNAMIXEL_SPEED_MAX_RPM            114
#define DYNAMIXEL_SPEED_MAX_RADPS          (DYNAMIXEL_SPEED_MAX_RPM * 2*(M_PI) / 60)
#define DYNAMIXEL_SPEED_FROM_RPM(rpm)      (DYNAMIXEL_SPEED_MAX_VALUE * (rpm) / DYNAMIXEL_SPEED_MAX_RPM)
// SI units (preffered convention)
#define DYNAMIXEL_SPEED_FROM_RADPS(radps)  (DYNAMIXEL_SPEED_MAX_VALUE * (radps) / DYNAMIXEL_SPEED_MAX_RADPS)
#define DYNAMIXEL_SPEED_TO_RADPS(value)    (DYNAMIXEL_SPEED_MAX_RADPS * (value) / DYNAMIXEL_SPEED_MAX_VALUE)
// Goal speed
#define DYNAMIXEL_GOAL_SPEED_DIRECTION_MASK (1 << 10)


#ifdef __cplusplus
}
#endif

