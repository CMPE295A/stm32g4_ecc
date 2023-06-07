/*
 * i2c.h
 *
 *  Created on: March 21, 2023
 *      Author: Jasdip Sekhon
 */

#include "driver_i2c.h"

//volatile i2c_state_e i2c_state = I2C__STATE_IDLE;
static uint8_t received_data;
i2c_state_e *i2c_state = I2C__STATE_IDLE;
/*
 * State Machine
 * START Condition -> SAD + Write -> Wait for ACK -> Send Register Address as Data -> Wait for ACK ->
 *  Send REPEAT_START Condition -> SAD + Read -> Wait for ACK -> Read data from slave -> Send ACK until NACK -> STOP
 * 1. Send START Condition
 * 2. Send Slave Address + Write bit set
 * 3. Wait for ACK
 * 4. Send register address as data
 * 5. Wait for ACK
 * 6. Send REPEAT_START Condition
 *  START->SAD+W->Wait for ACK-> Send Slave Register to read from as Data->Wait for ACK->REPEAT_START Condition->SAD+R->Wait for ACK->Read data from slave->ACK bytes, except NACK last byte-> STOP
 *
 * TODO:
 * 1. Rewrite ISR
 * 2. Add timeout to receive data
 * 3. Replace busy loops with callback functions
 * 4. Add better error handling (bus timeout, arbitration lost, overrun)
 */
void i2c__state_machine(i2c_state_e *i2c_state, uint8_t slave_address, uint8_t register_address, uint32_t bytes_to_read) {
	if (slave_address > 127) {
		I2C1->CR2 |= I2C_CR2_STOP;
		*i2c_state = I2C__STATE_ERROR;
	}
	switch(*i2c_state) {
	case I2C__STATE_IDLE:
		break;

	case I2C__STATE_START:
		while (!(I2C1->ISR & I2C_ISR_BUSY));
		I2C1->CR2 |= I2C_CR2_START; // Send start condition
		while (!(I2C1->ISR & I2C_ISR_BUSY));
		*i2c_state = I2C__STATE_SEND_SLAVE_ADDR_WITH_WRITE;
		break;

	case I2C__STATE_SEND_SLAVE_ADDR_WITH_WRITE:
		I2C1->CR2 = (slave_address << 1) & ~I2C_CR2_RD_WRN; // Send slave address with write bit set
		while (!(I2C1->ISR & I2C_ISR_ADDR)); // Wait for slave AWK
		if (I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_ARLO | I2C_ISR_BERR)) { // Slave NACK or arbitration lost
			I2C1->CR2 |= I2C_CR2_STOP;
			*i2c_state = I2C__STATE_ERROR;
		    break;
		}
		*i2c_state = I2C__STATE_SEND_REGISTER_ADDR;
		break;

	case I2C__STATE_SEND_REGISTER_ADDR:
		I2C1->TXDR = register_address;
		while (!(I2C1->ISR & I2C_ISR_TXE)); // Send register
		if (I2C1->ISR & (I2C_ISR_NACKF)) {  // Slave NACK
			I2C1->CR2 |= I2C_CR2_STOP;
			*i2c_state = I2C__STATE_ERROR;
			break;
		}
		*i2c_state = I2C__STATE_REPEATED_START_CONDITION;
		break;

	case I2C__STATE_REPEATED_START_CONDITION:
		    I2C1->CR2 |= I2C_CR2_START; // Send repeated start condition
		    while (!(I2C1->ISR & I2C_ISR_BUSY)); // Wait for start condition to be sent
		    *i2c_state = I2C__STATE_SEND_SLAVE_ADDR_WITH_READ;
		    break;

	case I2C__STATE_SEND_SLAVE_ADDR_WITH_READ:
		I2C1->CR2 = (slave_address << 1) | I2C_CR2_RD_WRN; // Send slave address with write bit set
		while (!(I2C1->ISR & I2C_ISR_ADDR)); // Wait for slave AWK
		if (I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_ARLO | I2C_ISR_BERR)) { // Slave NACK or arbitration lost
			I2C1->CR2 |= I2C_CR2_STOP;
			*i2c_state = I2C__STATE_ERROR;
		    break;
		}
		*i2c_state = I2C__STATE_RECEIVE_DATA;
		break;

	case I2C__STATE_RECEIVE_DATA:
		while (!(I2C1->ISR & I2C_ISR_RXNE)); // Wait for data to be received
		received_data = I2C1->RXDR; // Read data
		I2C1->CR2 &= ~I2C_CR2_NACK; // ACK
		*i2c_state = I2C__STATE_REPEATED_START_CONDITION;
//		bytes_to_read--;
//		if (bytes_to_read == 0) { // No more bytes to read
//			*i2c_state = I2C__STATE_STOP;
//		}
//		else if (bytes_to_read == 1) { // Send NACK if 1 last byte
//
//			I2C1->CR2 |= I2C_CR2_NACK;
//			*i2c_state = I2C__STATE_STOP;
//		}
//		else {  // More bytes to ACK
//			I2C1->CR2 &= ~I2C_CR2_NACK; // ACK
//			*i2c_state = I2C__STATE_RECEIVE_DATA;
//		}

		break;

	case I2C__STATE_STOP:
		I2C1->CR2 |= I2C_CR2_STOP;
		while (I2C1->CR2 & I2C_CR2_STOP); // Wait for STOP condition to be cleared internally
		while (!(I2C1->ISR & I2C_ISR_STOPF));
		I2C1->ICR &= ~(I2C_ICR_ADDRCF | I2C_ICR_NACKCF | I2C_ICR_STOPCF | I2C_ICR_BERRCF // Clear pending flags in ICR register
				| I2C_ICR_ARLOCF | I2C_ICR_OVRCF| I2C_ICR_PECCF | I2C_ICR_TIMOUTCF | I2C_ICR_ALERTCF);
		*i2c_state = I2C__STATE_IDLE;
		break;

	case I2C__STATE_ERROR:
		*i2c_state = I2C__STATE_IDLE;
		break;
	}
}

void I2C1_EV_IRQHandler(void) {
	// TODO
}

// I2C1 SDA = PB7, SCL = PA15
static void gpio_init(void) {
	const uint32_t i2c_mode = 0x2;
	const uint32_t pull_up = 0x1;
	const uint32_t clear = 0x3;
	const uint32_t AF4 = 0x4;
	const uint32_t open_drain = 0x1;

	GPIOB->MODER &= ~(clear << 14);
	GPIOB->MODER |= (i2c_mode << 14);      // Set PB7 Pin Function as I2C
	GPIOB->AFR[0] |= (AF4 << 28); 	       // Set alternate function to AF4 (SDA)
	GPIOB->OTYPER |= (open_drain << 7);    // Set output type to open-drain
	GPIOB->PUPDR &= ~(clear << 14);
	GPIOB->PUPDR |= (pull_up << 14);       // Enable pull-up resistor for PB7

	GPIOA->MODER &= ~(clear << 30);
	GPIOA->MODER |= (i2c_mode << 30);      // Set PA15 Pin Function as I2C
	GPIOA->AFR[1] |= (AF4 << 28);          // Set alternate function to AF4 (SCL)
	GPIOB->OTYPER |= (open_drain << 15);   // Set output type to open-drain
	GPIOA->PUPDR &= ~(clear << 30);
	GPIOA->PUPDR |= (pull_up << 30);       // Enable pull-up resistor for PA15
}

static void enable_GPIO_and_I2C_clock(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN; // enable peripheral clock for GPIO A and B
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN; // enable peripheral clock for I2C1
}

static void congifure_I2C_clock(void) {
	const uint32_t prescaler = 4;
	const uint32_t SCLDEL = 4;
	const uint32_t SDADEL = 2;
	const uint32_t SCLH = 67;
	const uint32_t SCLL = 134;

	// Configure I2C1_TIMINGR for 100KHz speed
	I2C1->TIMINGR = (prescaler << 28) |
					(SCLDEL << 20) |
					(SDADEL << 16) |
					(SCLH << 8) |
					(SCLL);
}

void i2c__initialize(void) {
	enable_GPIO_and_I2C_clock();
	gpio_init();

	I2C1->CR1 &= ~I2C_CR1_PE;
	congifure_I2C_clock();
	I2C1->CR1 &= ~I2C_CR1_ANFOFF;        // Enable analog filter
	I2C1->CR1 &= ~I2C_CR1_DNF;
	I2C1->CR1 |= 0x3 << I2C_CR1_DNF_Pos; // Set digital noise filter
	I2C1->CR1 &= ~I2C_CR1_NOSTRETCH; 	 // Enable clock stretching
	I2C1->CR2 &= ~I2C_CR2_NACK;			 // Enable NACK generation
	I2C1->OAR1 = 0x0;
	I2C1->OAR2 = 0x0;
	I2C1->CR2 = 0x0;
	I2C1->CR1 |= I2C_CR1_PE;

	// Enable interrupts for I2C events
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	NVIC_SetPriority(I2C1_EV_IRQn, 2);
	I2C1->CR1 |= I2C_CR1_RXIE; // enable Receive Data Register not empty interrupt
}


bool i2c__detect(void) {
    uint8_t address;
    uint8_t i2c_timeout_ms = 100;
    bool detected = false;

    for (address = 0; address < 128; address++) {
        I2C1->CR2 = (address << 1) | I2C_CR2_RD_WRN; // Start I2C transmission for write operation
        I2C1->CR2 |= I2C_CR2_START; // Send START condition
        while (!(I2C1->ISR & I2C_ISR_TC) && (i2c_timeout_ms--)); // Wait for the end of transaction
        if (!(I2C1->ISR & I2C_ISR_NACKF)) { // Check if the device responded
        	detected = true;
            printf("slave_address: %i\n", address);
        }
        I2C1->CR2 |= I2C_CR2_STOP; // Generate STOP condition
        while ((I2C1->ISR & I2C_ISR_STOPF) && (i2c_timeout_ms--)); // Wait for the end of the transaction
        i2c_timeout_ms = 100;
    }
    return detected;
}

uint8_t i2c__read_slave_data_state_machine(i2c_state_e *i2c_state, uint8_t slave_address, uint8_t register_address, uint32_t number_of_bytes) {
    *i2c_state = I2C__STATE_START;
    I2C1->CR1 |= I2C_CR1_RXIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE; // Enable I2C interrupts
    i2c__state_machine(i2c_state, slave_address, register_address, number_of_bytes); // Send data to slave
    while (i2c_state != I2C__STATE_IDLE); // Wait for completion
    I2C1->CR1 &= ~(I2C_CR1_RXIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE); // Disable I2C interrupts
    return received_data;
}

uint8_t i2c__read_slave_data(uint8_t slave_address, uint8_t register_address) {
	uint8_t data = 0;
	I2C1->CR2 = (slave_address << 1) | I2C_CR2_START | I2C_CR2_AUTOEND; // Send start condition and slave address with write bit
	while (!(I2C1->ISR & I2C_ISR_TXIS)); // Wait for end of address transmission
	I2C1->TXDR = register_address;       // Send register address
	while (!(I2C1->ISR & I2C_ISR_TC));   // Wait for end of data transmission
	I2C1->CR2 = (slave_address << 1) | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND; // Send start and request read
	while (!(I2C1->ISR & I2C_ISR_RXNE)); // Wait for end of address transmission
	data = I2C1->RXDR;
	return data;
}

void i2c__write_slave_data(uint8_t slave_address, uint8_t register_address, uint8_t data) {
    while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY); // Wait until I2C bus is ready
    I2C1->CR2 = (slave_address << 1) | I2C_CR2_AUTOEND | ((0x2 << I2C_CR2_NBYTES_Pos)); // Send start condition and slave address
    I2C1->CR2 |= I2C_CR2_START;
    while((I2C1->ISR & I2C_ISR_TXIS) == 0);  // Wait for TXIS flag to be set (transmit register address)
    I2C1->TXDR = register_address;                // Write the register address to the DR register
    while((I2C1->ISR & I2C_ISR_TXIS) == 0);  // Wait for TXIS flag to be set (transmit data)
    I2C1->TXDR = data; 						 // Write the data to the DR register
    while((I2C1->ISR & I2C_ISR_STOPF) == 0); // Wait for STOP flag to be set
    I2C1->ICR |= I2C_ICR_STOPCF;             // Clear the STOP flag by writing 0 to it
}


