#pragma once
#include "stdbool.h"
#include "stdint.h"

bool connect(void);
void publish(char *message, uint16_t byte_length);
