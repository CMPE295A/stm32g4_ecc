#include "driver_timer.h"
#include "stm32g431xx.h"
#include "stm32g4xx_hal.h"

static const uint32_t TIMER_CLOCK_HZ = 170000000;
static const uint32_t TIMER_MS_PER_SECOND = 1000;

typedef struct
{
	TIM_TypeDef *regs;
	volatile uint32_t ms_count;
	volatile uint32_t last_ms_count;
}timer_t;

timer_t timer[NUM_TIMER_DRIVERS] =
{
		{.regs = TIM2},
		{.regs = TIM3},
		{.regs = TIM4},
};

bool timer__initialize(uint8_t id)
{
	bool initialized = false;
	switch(id)
	{
	case DRIVER_TIMER2:
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
		NVIC->ISER[(TIM2_IRQn / 32)] |= (1 << (TIM2_IRQn % 32));
		NVIC->IP[TIM2_IRQn] |= 0x20;
		break;

	case DRIVER_TIMER3:
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
		NVIC->ISER[(TIM3_IRQn / 32)] |= (1 << (TIM3_IRQn % 32));
		break;

	case DRIVER_TIMER4:
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
		NVIC->ISER[(TIM4_IRQn / 32)] |= (1 << (TIM4_IRQn % 32));
			break;

	default:
		break;
	}

	if(id < NUM_TIMER_DRIVERS)
	{
		timer[id].ms_count = 0;
		timer[id].last_ms_count = 0;
		uint32_t divisor_ms = TIMER_CLOCK_HZ / TIMER_MS_PER_SECOND;

		// configure for
		timer[id].regs->CR1 |= TIM_CR1_URS;
		timer[id].regs->ARR = divisor_ms;
		timer[id].regs->DIER |= TIM_DIER_UIE;
		timer[id].regs->CR1 |= TIM_CR1_CEN;
		initialized = true;
	}

	return initialized;
}

bool timer__ms_elapsed(uint8_t id)
{
	bool elapsed = false;
	if(id < NUM_TIMER_DRIVERS)
	{
		elapsed = timer[id].ms_count != timer[id].last_ms_count;
		if(elapsed)
		{
			timer[id].last_ms_count = timer[id].ms_count;
		}
	}
	return elapsed;
}

uint32_t timer__get_ms(uint8_t id)
{
	uint32_t ms = 0;
	if(id < NUM_TIMER_DRIVERS)
	{
		ms = timer[id].ms_count;
	}
	return ms;
}

void timer__interrupt_handler(uint8_t id)
{
	timer[id].ms_count++;
	timer[id].regs->SR &= ~TIM_SR_UIF;
}

void TIM2_IRQHandler(void)
{
	timer__interrupt_handler(DRIVER_TIMER2);
}

void TIM3_IRQHandler(void)
{
	timer__interrupt_handler(DRIVER_TIMER3);
}

void TIM4_IRQHandler(void)
{
	timer__interrupt_handler(DRIVER_TIMER4);
}
