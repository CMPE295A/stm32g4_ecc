#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32g4xx_hal.h"

void sensor__init(void);
void sensor__get_accel(void);
void sensor__get_gyro(void);
void sensor_Error_Handler(void);

