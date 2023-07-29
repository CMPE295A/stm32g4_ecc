/*
 * driver_timer_1.c
 *
 *  Created on: July 11, 2023
 *      Author: Jasdip Sekhon
 */

#include "driver_timer_1.h"

/*
 * Resolution: 16-bits (auto-reolad counter)
 * Max Resolution: 2^16 = 65536 counts/cycle (max resolution)
 * PWM Frequency = 170MHz/65536 = 2593Hz = 2.593KHz cycles/sec (min freq/highest resolution)
 * resolution = TIM1 clock freq / (TIM1->ARR)
 * Resolution and frequency are inversely proportional
 * Prescaler: divides the counter clock frequency by any factor between 1 and 65536
 */

static const uint32_t timer_clock_frequency_Hz = 170000000;

static void set_gpio_pins(void) {
	GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk);
	GPIOA->MODER |= (GPIO_MODER_MODE8_1);
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL8_Msk);
	GPIOA->AFR[1] |= (6U << GPIO_AFRH_AFSEL8_Pos);

}

static void start_clock(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // Enable GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable TIM1 clock
}
static void timer_1__set_pwm_configuration(void) {
    // Configure channel 1 for PWM mode 1
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;

    // Set the pulse width value
    TIM1->CCR1 = 25;

    // Set the output polarity to active high
    TIM1->CCER &= ~TIM_CCER_CC1P;

    // Set the complementary output polarity to active high (if needed)
    TIM1->CCER &= ~TIM_CCER_CC1NP;

    // Disable fast mode
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1FE;

    // Set the output idle state to reset
    TIM1->CR2 &= ~TIM_CR2_OIS1;
}

void timer_1__initialize(void) {
	set_gpio_pins();
	start_clock();
	TIM1->CR1 = 0;
	TIM1->CR2 = 0;
	TIM1->CCMR1 = 0;
	TIM1->CCER = 0;
	TIM1->PSC = 0;
	TIM1->ARR = 0;
	TIM1->SR = 0;
	TIM1->PSC = 0;
	TIM1->ARR = 255; // auto-reload value
	TIM1->CR1 &= ~TIM_CR1_CKD; // Set the clock division to Div1
	TIM1->RCR = 0;
	TIM1->CR1 &= ~TIM_CR1_DIR; // set to count up

	// Configure the master mode settings
	TIM1->CR2 &= ~TIM_CR2_MMS;

	// Disable the master-slave mode
	TIM1->SMCR &= ~TIM_SMCR_MSM;

	timer_1__set_pwm_configuration();

	TIM1->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos) | (0x1 << TIM_CCMR1_OC1PE_Pos); // Mode 1; channel 1 active when TIMx_CNT<TIMx_CCR1
	TIM1->EGR |= TIM_EGR_UG; // Reinitialize the counter
	TIM1->CR1 |= TIM_CR1_CEN | (0x1 << TIM_CR1_ARPE_Pos); // Enable TIM1 counter; Auto-reload pre-load enable
//	TIM1->DIER |= TIM_DIER_UIE; // Enable timer interrupt
}

void timer_1__set_duty_cycle(float duty_cycle_in_percent){
	uint32_t counts_per_cycle = TIM1->ARR + 1;
	uint32_t duty_cycle_raw = (duty_cycle_in_percent * counts_per_cycle) / 100;
	TIM1->CCR1 = duty_cycle_raw;
}

void timer_1__set_frequency(float pwm_frequency_Hz) {
	uint32_t counts_per_cycle = (timer_clock_frequency_Hz / pwm_frequency_Hz) - 1;
	TIM1->ARR = counts_per_cycle;
}

void TIM1_IRQHandler(void) {
	// Handle the timer interrupt here
	TIM1->SR &= ~TIM_SR_UIF;
}

