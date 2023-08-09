#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32g4xx_hal.h"

typedef struct {
    float roll;
    float pitch;
    float yaw;
    float horizontal;
    float vertical;
    float lateral;
}sensor_t;

void sensor__init(void);
void sensor__get_accel(sensor_t *sensor);
void sensor__get_gyro(sensor_t *sensor);

