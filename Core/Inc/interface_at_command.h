#pragma once

#include <stdint.h>
#include <stdbool.h>

enum
{
	AT_INTERFACE_ECHO_OFF = 0,
	AT_INTERFACE_TEST,
	AT_INTERFACE_MODE,
	AT_INTERFACE_SET_DHCP,
	AT_INTERFACE_LIST_AP,
	AT_INTERFACE_SSID,
	AT_INTERFACE_CONNECT,
	AT_INTERFACE_MQTT_SETUP,
	AT_INTERFACE_MQTT_CONNECT,
	AT_INTERFACE_NETWORK_UP,
	AT_INTERFACE_FAULT,

	AT_INTERFACE_NUM_STATES
};

bool at_interface__initialize(void);
void at_interface__process(void);
void at_interface__get_packet(char *packet, uint16_t size);
void at_interface__send_string(char *str);
void at_interface__send_test_string(void);

