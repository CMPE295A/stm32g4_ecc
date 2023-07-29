/*
 * driver_timer_1.h
 *
 *  Created on: Jul 11, 2023
 *      Author: Jasdip Sekhon
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "stm32g431xx.h"

void timer_1__initialize(void);

void timer_1__set_duty_cycle(float duty_cycle_in_percent);

void timer_1__set_frequency(float pwm_frequency_Hz);

