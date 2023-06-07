#pragma once

#include <stdint.h>
#include <stdbool.h>

enum
{
	INTERFACE_ESP8266_ECHO_OFF = 0,
	INTERFACE_ESP8266_TEST,
	INTERFACE_ESP8266_MODE,
	INTERFACE_ESP8266_LIST_AP,
	INTERFACE_ESP8266_SSID,
	INTERFACE_ESP8266_CONNECT,
	INTERFACE_ESP8266_NETWORK_UP,
	INTERFACE_ESP8266_FAULT,

	INTERFACE_ESP8266_NUM_STATES
};

bool esp8266__initialize(void);
void esp8266__process(void);
void esp8266__get_packet(char *packet, uint16_t size);
void esp8266__send_string(char *str);
void esp8266__send_test_string(void);

