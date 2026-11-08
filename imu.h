#pragma once

#include <Arduino.h>

// QMI8658 on the Waveshare I2C bus. No interrupt pin: poll only.
bool imuBegin();
bool imuOk();
void imuPoll(uint32_t nowMs, uint16_t intervalMs);
bool imuShakeEdge();          // one-shot impulse, consumes the event
uint16_t imuTakeSteps();      // delta since the last call
float imuLastMagG();          // last |a| in g, 0 if unknown
float imuLastGyroDps();       // letzte |gyro| in deg/s
uint8_t imuAddr();            // 0 if not found
uint32_t imuPedometer();      // raw counter of the chip
uint32_t imuSoftwarePedometer(); // own counter from acceleration values
