#pragma once

#include <stdint.h>
#include <stdbool.h>

#define AT_INTERFACE_SEND_DELAY_MS 500

enum
{
	AT_INTERFACE_INIT = 0,
	AT_INTERFACE_ECHO_OFF,
	AT_INTERFACE_TEST,
	AT_INTERFACE_MODE,
	AT_INTERFACE_SET_DHCP,
	AT_INTERFACE_LIST_AP,
	AT_INTERFACE_SSID,
	AT_INTERFACE_SSL_CONFIG,
	AT_INTERFACE_CONNECT,
	AT_INTERFACE_CLA_CERT_SET,
	AT_INTERFACE_CLA_CERT_SEND,
	AT_INTERFACE_CLIENT_CERT_SET,
	AT_INTERFACE_CLIENT_CERT_SEND,
	AT_INTERFACE_PRIVATE_KEY_SET,
	AT_INTERFACE_PRIVATE_KEY_SEND,
	AT_INTERFACE_MQTT_SET_TOPIC,
	AT_INTERFACE_MQTT_SETUP,
	AT_INTERFACE_MQTT_CONNECT,
	AT_INTERFACE_NETWORK_UP,
	AT_INTERFACE_GET_SHARED_SECRET,
	AT_INTERFACE_MQTT_DISCONNECT,
	AT_INTERFACE_FAULT,

	AT_INTERFACE_NUM_STATES
};

bool at_interface__initialize(void);
void at_interface__process(bool ms_elapsed);
void at_interface__get_packet(char *packet, uint16_t size);
void at_interface__send_string(char *str);
void at_interface__send_test_string(void);
bool at_interface__publish_string(char *str);
void at_interface__publish_test(void);
