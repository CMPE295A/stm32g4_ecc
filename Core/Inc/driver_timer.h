#pragma once

#include <stdint.h>
#include <stdbool.h>

enum
{
	DRIVER_TIMER2,
	DRIVER_TIMER3,
	DRIVER_TIMER4,

	NUM_TIMER_DRIVERS
};

bool timer__initialize(uint8_t id);
bool timer__ms_elapsed(uint8_t id);
uint32_t timer__get_ms(uint8_t id);

void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
