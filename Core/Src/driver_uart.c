#include "driver_uart.h"
#include "stm32g431xx.h"
#include "stm32g4xx_hal.h"

static const uint32_t UART_CLOCK_HZ = 170000000;
#define BUFFER_SIZE 1028
#define FIFO_SIZE 8

void rx_interrupt_handler(uint8_t id);
void tx_interrupt_handler(uint8_t id);

typedef struct
{
	volatile uint8_t buffer[BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
	volatile uint16_t count;
}buffer_t;

typedef struct
{
	USART_TypeDef *regs;
	buffer_t rx;
	buffer_t tx;
}uart_t;

static uart_t uart[NUM_UART_DRIVERS] =
{
		{.regs = USART1},
		{.regs = USART2},
		{.regs = USART3}
};

bool uart__initialize(uint8_t id, uint32_t baudrate)
{
	bool initialized = false;
	switch (id)
	{
	case DRIVER_UART1:
	{
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		NVIC->ISER[(USART1_IRQn / 32)] |= (1 << (USART1_IRQn % 32));
		break;
	}
	case DRIVER_UART2:
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
		NVIC->ISER[(USART2_IRQn / 32)] |= (1 << (USART2_IRQn % 32));
		break;
	}
	case DRIVER_UART3:
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
		NVIC->ISER[(USART3_IRQn / 32)] |= (1 << (USART3_IRQn % 32));
		break;
	}
	}

	if(id < NUM_UART_DRIVERS)
	{
		uart[id].regs->CR1 = 0;
		uart[id].regs->CR1 |= USART_CR1_RXNEIE | USART_CR1_TXEIE_TXFNFIE | USART_CR1_TE | USART_CR1_RE;
		uart[id].regs->BRR = (UART_CLOCK_HZ / baudrate);
		uart[id].regs->CR2 = 0;
		uart[id].regs->CR3 = 0;

		uart[id].regs->CR1 |= USART_CR1_UE;

		initialized = true;
	}
	return initialized;
}

uint8_t uart__get_char(uint8_t id)
{
	uint8_t c = NO_CHAR_AVAILABLE;
	if(uart[id].rx.count > 0)
	{
		c = uart[id].rx.buffer[uart[id].rx.tail];
		if(uart[id].rx.tail == (BUFFER_SIZE - 1))
		{
			uart[id].rx.tail = 0;
		}
		else
		{
			uart[id].rx.tail++;
		}
		uart[id].rx.count--;
	}
	if(uart[id].regs->ISR & USART_ISR_IDLE)
	{
		uart[id].regs->ICR |= USART_ICR_IDLECF;
	}
	if(uart[id].regs->ISR & USART_ISR_FE)
	{
		uart[id].regs->ICR |= USART_ICR_FECF;
	}
	return c;
}

void uart__put(uint8_t id, uint8_t *buff, uint16_t size)
{
	for(uint16_t i = 0; i < size; i++)
	{
		if(uart[id].tx.count == BUFFER_SIZE)
		{
			break;
		}
		uart[id].tx.buffer[uart[id].tx.head] = *buff++;
		if(uart[id].tx.head == (BUFFER_SIZE - 1))
		{
			uart[id].tx.head = 0;
		}
		else
		{
			uart[id].tx.head++;
		}
		uart[id].tx.count++;
	}

	// enable interrupt
	uart[id].regs->CR1 |= USART_CR1_TXEIE_TXFNFIE;
}

// when rx fifo not empty, add chars to buffer
// if buffer is full, drop chars
void rx_interrupt_handler(uint8_t id)
{
	uint8_t c = uart[id].regs->RDR;
	if(uart[id].rx.count < BUFFER_SIZE)
	{
		uart[id].rx.buffer[uart[id].rx.head] = c;
		if(uart[id].rx.head == (BUFFER_SIZE - 1))
		{
			uart[id].rx.head = 0;
		}
		else
		{
			uart[id].rx.head++;
		}
		uart[id].rx.count++;
	}
}

// if chars in transmit buffer, add to transmit fifo
// if transmit buffer empty, disable interrupt
void tx_interrupt_handler(uint8_t id)
{
	while(uart[id].regs->ISR & USART_ISR_TXE_TXFNF)
	{
		if(uart[id].tx.count == 0)
		{
			// disable tx interrupt
			uart[id].regs->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
			break;
		}
		else
		{
			uart[id].regs->TDR = uart[id].tx.buffer[uart[id].tx.tail];
			if(uart[id].tx.tail == (BUFFER_SIZE - 1))
			{
				uart[id].tx.tail = 0;
			}
			else
			{
				uart[id].tx.tail++;
			}
			uart[id].tx.count--;
		}
	}
}

void uart__interrupt_handler(uint8_t id)
{
	volatile uint32_t status = uart[id].regs->ISR;
	if(status & USART_ISR_RXNE_RXFNE)
	{
		// Read fifo until empty to clear interrupt
		rx_interrupt_handler(id);
	}
	if(status & USART_ISR_TXE)
	{
		// Write to TDR to clear fifo
		tx_interrupt_handler(id);
	}
	uart[id].regs->ICR |= 0x00123BFF;
}

void USART1_IRQHandler(void)
{
	uart__interrupt_handler(DRIVER_UART1);
}

void USART2_IRQHandler(void)
{
	uart__interrupt_handler(DRIVER_UART2);
}

void USART3_IRQHandler(void)
{
	uart__interrupt_handler(DRIVER_UART3);
}
