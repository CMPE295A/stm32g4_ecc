/*
 * i2c.h
 *
 *  Created on: March 21, 2023
 *      Author: Jasdip Sekhon
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "stm32g431xx.h"

typedef enum {
  I2C__STATE_IDLE = 0,
  I2C__STATE_START,
  I2C__STATE_SEND_SLAVE_ADDR_WITH_WRITE,
  I2C__STATE_SEND_SLAVE_ADDR_WITH_READ,
  I2C__STATE_SEND_REGISTER_ADDR,
  I2C__STATE_REPEATED_START_CONDITION,
  I2C__STATE_RECEIVE_DATA,
  I2C__STATE_STOP,
  I2C__STATE_ERROR,
} i2c_state_e;

void i2c__initialize(void);

/**
 * @returns true if the I2C device at the given address responds back with an ACK
 */
bool i2c__detect(void);

/**
 * Reads multiple registers of the slave_address
 */
uint8_t i2c__read_slave_data(uint8_t slave_address, uint8_t register_address);

uint8_t i2c__read_slave_data_state_machine(i2c_state_e *i2c_state, uint8_t slave_address, uint8_t register_address, uint32_t number_of_bytes);

/**
 * Writes multiple registers of the slave_address
 */
void i2c__write_slave_data(uint8_t slave_address, uint8_t register_address, uint8_t data);

void i2c__state_machine(i2c_state_e *i2c_state, uint8_t slave_address, uint8_t register_address, uint32_t bytes_to_read);

void I2C1_EV_IRQHandler(void);
